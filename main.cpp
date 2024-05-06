#include <vector>
#include <utility>
#include <limits>
#include <algorithm>
#include <iostream>

using namespace std;

class Node;

class Rect {
public:
  float x_min;
  float x_max;
  float y_min;
  float y_max;

  Rect() {
    x_min = 0;
    x_max = 0;
    y_min = 0;
    y_max = 0;
  }
  Rect(float x_min, float y_min, float x_max, float y_max) {
    this->x_min = x_min;
    this->y_min = y_min;
    this->x_max = x_max;
    this->y_max = y_max;
  }
  float area() {
    return (x_max - x_min) * (y_max - y_min);
  }
  float enlargement(Rect E) {
    Rect combined(min(E.x_min, x_min), 
                  min(E.y_min, y_min),
                  max(E.x_max, x_max), 
                  max(E.y_max, y_max));
    return combined.area() - area();
  }
  Rect combination(Rect E) {
    Rect combined(min(E.x_min, x_min), 
                  min(E.y_min, y_min),
                  max(E.x_max, x_max), 
                  max(E.y_max, y_max));
    return combined;
  }

  bool overlaps(Rect E) {
    return (x_min <= E.x_max && 
            x_max >= E.x_min && 
            y_min <= E.y_max && 
            y_max >= E.y_min);
  }
};

class Node {
public:
  vector<Node> children;
  Rect rect;
  Node* father;

  Node() {
    rect = Rect();
  }

  Node(Rect E) {
    rect = E;
  }
  bool isLeaf() {
    if (children[0].children.empty()) {
      return true;
    }
    return false;
  }

  void recalculateRectangle() {
    float x_min = numeric_limits<float>::infinity();
    float x_max = -numeric_limits<float>::infinity();
    float y_min = numeric_limits<float>::infinity();
    float y_max = -numeric_limits<float>::infinity();
    for (int i = 0; i < children.size(); i++) {
      x_min = min(x_min, min(children[i].rect.x_min, children[i].rect.x_max));
      y_min = min(y_min, min(children[i].rect.y_min, children[i].rect.y_max));
      x_max = max(x_max, max(children[i].rect.x_min, children[i].rect.x_max));
      y_max = max(y_max, max(children[i].rect.y_min, children[i].rect.y_max));
    }
    rect = Rect(x_min, y_min, x_max, y_max);
  }

  bool operator==(const Node& rhs) {
    return rect.x_min == rhs.rect.x_min &&
           rect.y_min == rhs.rect.y_min &&
           rect.x_max == rhs.rect.x_max &&
           rect.y_max == rhs.rect.y_max;
  }
};

class RTree {
public:
  Node root;
  int M;
  int m;
  RTree(int M) {
    cout << "---------- CREATING TREE ----------" << endl;
    this->M = M;
    m = M / 2;
    root = Node();
    root.father = nullptr;
    cout << "----- FINISHED CREATING TREE ------" << endl << endl;
  }

  void Insert(Rect E) {
    //! I1 Find position for new record.
    cout << "--------- INSERTING NODE ----------" << endl;
    Node* leafNode = &root;
    if (root.children.empty()) {
      Node newNode(E);
      newNode.father = leafNode;
      leafNode->children.push_back(newNode);
      root.recalculateRectangle();
      cout << "First insert in root" << endl;
      // printNodeStatus(leafNode, 0);
      cout << "------- FINISHED INSERTING --------" << endl << endl;
      printTreeStatus();
      return;
    }
    ChooseLeaf(E, leafNode);
    Node newChild(E);
    Insert(leafNode, newChild);
    printTreeStatus();
    return;
  }

  void Insert(Node*& father, Node& newChild) {

    //! I2 Add record to leaf node.
    newChild.father = father;
    father->children.push_back(newChild);


    Node *splitted = nullptr;
    if (father->children.size() == M + 1) {
      splitNode(*father, splitted);
    }

    //! I3 Propagate changes upward
    adjustTree(father, splitted);

    cout << "------- FINISHED INSERTING --------" << endl << endl;
    return;
  }

  void ChooseLeaf(Rect E, Node*& N) {
    //! CL1 Initialize
    cout << "    " << "------- SEARCHING FOR LEAF --------" << endl;
    //! CL2 Leaf check
    while (!N->isLeaf()) {
    //! CL3 Choose subtree
      float lessEnlargement = numeric_limits<float>::infinity();
      Node* currentBest = nullptr; 
      for (int i = 0; i < N->children.size(); i++) {
        if (N->children[i].rect.enlargement(E) < lessEnlargement) {
          currentBest = &(N->children[i]);
          lessEnlargement = N->children[i].rect.enlargement(E);
        }
      }

    //! CL4 Descend until a leaf is reached
      N = currentBest;
    }
    // printNodeStatus(N, 0);
    cout << "    " << "------- FINISHED SEARCHING --------" << endl << endl;
  }

  void splitNode(Node& node, Node*& splitted) {
    //! QS1 Pick first entry for each group
    cout << "    " << "--------- SPLITTING NODE ----------" << endl;
    vector<Node> rest = node.children;

    pair<Node, Node> seeds = pickSeeds(node);
    Node* E1 = &seeds.first;
    Node* E2 = &seeds.second;
    cout << "    " << "Seeds: " << endl;
    cout << "    " << "Seed 1: " << "(" << E1->rect.x_min << ", " << E1->rect.y_min << "), " << "(" << E1->rect.x_max << ", " << E1->rect.y_max << ")" << endl;
    cout << "    " << "Seed 2: " << "(" << E2->rect.x_min << ", " << E2->rect.y_min << "), " << "(" << E2->rect.x_max << ", " << E2->rect.y_max << ")" << endl;
    cout << endl;


    for (int i = 0; i < node.children.size(); i++) {
      if (node.children[i] == *E2) {
        node.children.erase(node.children.begin() + i);
        break;
      }
    }

    for (int i = 0; i < rest.size(); i++) {
      if (rest[i] == *E1) {
        rest.erase(rest.begin() + i);
        break;
      }
    }

    for (int i = 0; i < rest.size(); i++) {
      if (rest[i] == *E2) {
        rest.erase(rest.begin() + i);
        break;
      }
    }

    splitted = new Node();
    splitted->children.push_back(*E2);

    //! QS2 Check if done
    while (!rest.empty()) {
      if (node.children.size() >= m && (splitted->children.size() + rest.size() == m)) {
        cout << "    " << "Inserting the rest in first node!" << endl;
        for (int i = 0; i < rest.size(); i++) {
          splitted->children.push_back(rest.back());
          rest.pop_back();
        }
      }
      else if (splitted->children.size() >= m && (node.children.size() + rest.size() == m)) {
        cout << "    " << "Inserting the rest in second node!" << endl;
        for (int i = 0; i < rest.size(); i++) {
          node.children.push_back(rest.back());
          rest.pop_back();
        }
      }

    //! QS3 Select entry to assign
      else {
        Node next = pickNext(node, *splitted, rest);
        cout << "    " << "Next to be inserted: (" << next.rect.x_min << ", " << next.rect.y_min << "), " << "(" << next.rect.x_max << ", " << next.rect.y_max << ")" << endl;
        if (node.rect.enlargement(next.rect) > splitted->rect.enlargement(next.rect)) {
          for (int i = 0; i < node.children.size(); i++) {
            if (node.children[i].rect.x_min == next.rect.x_min &&
                node.children[i].rect.x_max == next.rect.x_max &&
                node.children[i].rect.y_min == next.rect.y_min &&
                node.children[i].rect.y_max == next.rect.y_max) {
              node.children.erase(node.children.begin() + i);
              break;
            }
          }
          splitted->children.push_back(next);
          cout << "    " << "Inserting next into second node!" << endl;
        }
        else if (node.rect.enlargement(next.rect) == splitted->rect.enlargement(next.rect)){
          if (node.rect.area() > splitted->rect.area()) {
            for (int i = 0; i < node.children.size(); i++) {
              if (node.children[i].rect.x_min == next.rect.x_min &&
                  node.children[i].rect.x_max == next.rect.x_max &&
                  node.children[i].rect.y_min == next.rect.y_min &&
                  node.children[i].rect.y_max == next.rect.y_max) {
                node.children.erase(node.children.begin() + i);
                break;
              }
            }
            splitted->children.push_back(next);
            cout << "    " << "Inserting next into second node!" << endl;
          }
          else if (node.children.size() > splitted->children.size()){
            for (int i = 0; i < node.children.size(); i++) {
              if (node.children[i].rect.x_min == next.rect.x_min &&
                  node.children[i].rect.x_max == next.rect.x_max &&
                  node.children[i].rect.y_min == next.rect.y_min &&
                  node.children[i].rect.y_max == next.rect.y_max) {
                node.children.erase(node.children.begin() + i);
                break;
              }
            }
            splitted->children.push_back(next);
            cout << "    " << "Inserting next into second node!" << endl;
          }
        }
        else {
          cout << "    " << "Inserting next into first node!" << endl;
        }
        for (int j = 0; j < rest.size(); j++) {
          if (rest[j].rect.x_min == next.rect.x_min &&
              rest[j].rect.x_max == next.rect.x_max &&
              rest[j].rect.y_min == next.rect.y_min &&
              rest[j].rect.y_max == next.rect.y_max) {
            rest.erase(rest.begin() + j);
            break;
          }
        }
      }
    }


    cout << endl;
    cout << "    Result nodes:" << endl;
    // printNodeStatus(&node, 0);
    // printNodeStatus(splitted, 0);
    cout << "    " << "------- FINISHED SPLITTING --------" << endl;
  }

  pair<Node, Node> pickSeeds(Node& node) {
    //! PS1 Calculate inefficiency of grouping entries together
    cout << "    " << "    " << "---------- PICKING SEEDS ----------" << endl;
    cout << "    " << "    " << "Picking seeds from seeds candidates:" << endl;
    for (int i = 0; i < node.children.size(); i++) {
      cout << "    " << "    " << "(" << node.children[i].rect.x_min << ", " << node.children[i].rect.y_min << "), " << "(" << node.children[i].rect.x_max << ", " << node.children[i].rect.y_max << ")" << endl;
    }
    cout << endl;
    Node E1;
    Node E2;
    float d = -numeric_limits<float>::infinity();
    int index;
    int jndex; 
    for (int i = 0; i < node.children.size(); i++) {
      for (int j = i + 1; j < node.children.size(); j++) {
        E1 = node.children[i];
        E2 = node.children[j];
        Rect J = E1.rect.combination(E2.rect);
        if (d < J.area() - E1.rect.area() - E2.rect.area()) {
          d = J.area() - E1.rect.area() - E2.rect.area();
          index = i;
          jndex = j;
        }
      }
    }

    //! PS2 Choose the most wasteful pair
    E1 = node.children[index];
    E2 = node.children[jndex];


    cout << "    " << "    " << "-------- FINISHED PICKING ---------" << endl;
    return make_pair(E1, E2);
  }

  Node pickNext(Node &node, Node &splitted, vector<Node> rest) {
    //! PN1 Determine cost of putting each entry in each group

    cout << "    " << "    " << "---------- PICKING NEXT -----------" << endl;
    node.recalculateRectangle();
    splitted.recalculateRectangle();
    int index;
    float maximumDifference = -numeric_limits<float>::infinity();
    for (int i = 0; i < rest.size(); i++) {
      float d1 = node.rect.enlargement(rest[i].rect);
      float d2 = splitted.rect.enlargement(rest[i].rect);

    //! PN2 Find entry with greatest preference for one group
      if (abs(d1 - d2) > maximumDifference) {
        index = i;
        maximumDifference = abs(d1 - d2);
      }
    }
    // printNodeStatus(&rest[index], 0);
    cout << "    " << "    " << "-------- FINISHED PICKING ---------" << endl;
    return (rest[index]);
  }

  void adjustTree(Node*& leaf, Node*& splitted) {
    cout << "    " << "--------- ADJUSTING TREE ----------" << endl;
    //! AT1 Initialize
    reAdjustAllChildren();
    leaf->recalculateRectangle();

    cout << "    " << "Nodes to be adjusted: " << endl;
    // printNodeStatus(leaf, 0);
    if (splitted) {
      splitted->recalculateRectangle();
      // printNodeStatus(splitted, 0);
    }
    cout << endl;
    //! AT2 Check if done

    if (*leaf == root) {
      cout << "    " << "Adjusting the root ";
      if (splitted) {
        cout << "after splitting" << endl;
    //! AT3 Adjust covering rectangle in parent

        Rect combined_rect = leaf->rect.combination(splitted->rect);
        Node P = Node(combined_rect);
        P.children.push_back(*leaf);
        P.children.push_back(*splitted);
    //! AT4 Propagate node split upward

        P.recalculateRectangle();

        root = P;
        leaf->father = &root;
        splitted->father = &root;
        root.father = nullptr;
        cout << "    " << "New root created:" << endl;
        // printNodeStatus(&root, 0);
        cout << endl;
        cout << "    " << "Nodes:" << endl;
        for (int i = 0; i < root.children.size(); i++) {
          // printNodeStatus(&root.children[i], 0);
        }
    //! AT5 Move up to next level

      }
      else {
        cout << endl;
        root.recalculateRectangle();
      }
    }
    else {
      cout << "    " << "Adjusting a node ";
      if (splitted) {
        cout << "after splitting" << endl;

    //! AT3 Adjust covering rectangle in parent
          Node* P = leaf->father;
          P->recalculateRectangle();

    //! AT4 Propagate node split upward
          Insert(P, *splitted);

    //! AT5 Move up to next level

      }
      else {
        cout << endl;
        while (leaf != &root) {
          leaf = leaf->father;
          leaf->recalculateRectangle();
        }
      }
    }

    cout << "    " << "------- FINISHED ADJUSTING --------" << endl;
    return;

  }

  vector<Rect> Search(Rect S) {
    vector<Rect> sol;
    Search(&root, S, sol);
    return sol;
  }
  void Search(Node* T, Rect S, vector<Rect>& sol) {
    //! S1 Search subtrees

    if (T->isLeaf()) {
      for (int i = 0; i < T->children.size(); i++) {
        if (T->children[i].rect.overlaps(S)) {
          sol.push_back(T->children[i].rect);
        }
      }
    }
    else {
      for (int i = 0; i < T->children.size(); i++) {
        if (T->children[i].rect.overlaps(S)) {
          Search(&T->children[i], S, sol);
        }
      }
    }
  }

  void printValues() {
    printValuesHelper(&root, 0);
  }

  void printValuesHelper(Node* node, int depth) {
    string indentation(depth, '\t');
    cout << indentation << "Rectangle: (" << node->rect.x_min << ", " << node->rect.y_min << "), ("
         << node->rect.x_max << ", " << node->rect.y_max << ")" << endl;
    for (int i = 0; i < node->children.size(); i++) {
        cout << indentation;
        printValuesHelper(&node->children[i], depth + 1);
    }
  }

  void printTreeStatus() {
    printNodeStatus(&root, 1);
  }
  void printNodeStatus(Node* node, int flag) {
    cout << "    " << "Node status: " << "(" << node->rect.x_min << ", " << node->rect.y_min << "), " << "(" << node->rect.x_max << ", " << node->rect.y_max << ")" << endl;
    if (node->father) {
        cout << "        " << "Father status: (" << node->father->rect.x_min << ", " << node->father->rect.y_min << "), " << "(" << node->father->rect.x_max << ", " << node->father->rect.y_max << ")" << endl;
    }
    cout << "        " << "Children status:" << endl;
    for (int i = 0; i < node->children.size(); i++) {
      cout << "        " << "(" << node->children[i].rect.x_min << ", " << node->children[i].rect.y_min << "), " << "(" << node->children[i].rect.x_max << ", " << node->children[i].rect.y_max << ")" << endl;
      if (node ->children[i].father) {
        cout << "            " << "Father of children status: (" << node->children[i].father->rect.x_min << ", " << node->children[i].father->rect.y_min << "), " << "(" << node->children[i].father->rect.x_max << ", " << node->children[i].father->rect.y_max << ")" << endl;
      }
    }
    if (flag == 1) {
      for (int i = 0; i < node->children.size(); i++) {
        printNodeStatus(&node->children[i], flag);
      }
    }
  }
  void reAdjustAllChildren() {
    reAdjustAllChildren(&root);
  }
  void reAdjustAllChildren(Node* node) {
    for (int i = 0; i < node->children.size(); i++) {
      node->children[i].father = node;
      // Llamada recursiva para reajustar los hijos de cada hijo
      reAdjustAllChildren(&(node->children[i]));
    }
  }
};

int main() {
  int M = 2;
  RTree tree(M);
  tree.Insert(Rect(20, 43, 20, 59));
  cout << endl;
  tree.Insert(Rect(48, 58, 50, 67));
  cout << endl;
  tree.Insert(Rect(74, 64, 105, 68));
  cout << endl;
  tree.Insert(Rect(83, 54, 40, 104));
  cout << endl;
  tree.Insert(Rect(20, 43, 20, 59));
  cout << endl;
  tree.Insert(Rect(48, 67, 105, 68));

  tree.printValues();
}

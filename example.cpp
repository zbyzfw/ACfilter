#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include <fstream>
// windows控制台格式化
#include <Windows.h>
// 使用pybind11
#include <pybind11/pybind11.h>
namespace py = pybind11;

using namespace std;
using namespace pybind11::literals;


// 定义节点类
struct node {
  vector<node *> *nextNodeVector = new vector<node *>; // 创建node类型下一个节点指针
  node *failNode = nullptr; //
  char w;
  bool hasEnd = false;
  int wordNum = 0;
  node(char w) : w(w) {} // 构造函数， 初始化列表方式

  // 获取下一个节点
  node *findNextNode(char w) {
    // 使用auto节省容器iterator的声明
    for (auto v : *nextNodeVector)
      if (v->w == w)
        return v;

      return nullptr;
  }
};

void WordFilterNormal(const string &input, const vector<string> &word_list, string &output, vector<string> &words) {
  output = input;
  if (input.empty())
    return;

  for (auto w : word_list) {
    if (w.length() > input.length())
      continue;

    auto pos = input.find(w);
    while (pos != string::npos) {
      // output.replace(pos, w.length(), w.length(), '*');
      words.emplace_back(w);
      pos = input.find(w, pos + 1);
    }
  }
}

// 接收一个词典链表和一个链表指针
void CreateWordTree(const vector<string> &word_list, node *root) {
  if (root == nullptr)
    return;

  for (auto w : word_list) {
    if (w.empty())
      continue;

    auto currentNode = root;
    int index = 0;
    while (index < w.length()) {
      node *find = currentNode->findNextNode(w[index]);
      // 按单个词的长度遍历，若不为空指针，切换到当前节点
      if (find != nullptr)
        currentNode = find;
      else {
        // 给节点赋值
        auto newNode = new node(w[index]);
        newNode->nextNodeVector->reserve(3);
        currentNode->nextNodeVector->emplace_back(newNode);
        currentNode = newNode;
      }

      index++;
      if (index == w.length()) {
        currentNode->hasEnd = true;
        // (int)将后方的值转换为int
        currentNode->wordNum = (int) w.length();
      }
    }
  }
}

// AC自动机，增加匹配失败后转到失败节点的功能
void CreateACAutomation(node *root) {
  if (root == nullptr)
    return;

  queue<node *> q;
  // 遍历所有节点，将一级节点的失败节点都赋值为root，并都存入队列
  for (auto v : *root->nextNodeVector) {
    v->failNode = root;
    q.push(v);
  }

  while (!q.empty()) {
    auto currentNode = q.front();
    q.pop();
    // 弹出开头的节点并遍历子树
    for (auto v : *currentNode->nextNodeVector) {
      auto failNode = currentNode->failNode;
      // 若当前节点的failNode不为空，从failNode的子节点树中查找（第一轮遍历failNode都为根节点）
      while (failNode != nullptr) {
        node *find = failNode->findNextNode(v->w);
        // 如果找到了相同的节点
        if (find != nullptr) {
          v->failNode = find;
          break;
        }
        // 没找到就判断failNode的failNode
        failNode = failNode->failNode;
      }

      if (failNode == nullptr)
        v->failNode = root;

      q.push(v);
    }
  }
}

void WordFilterDFA(const string &input, node *root, string &output, vector<string> &words) {
  output = input;
  if (input.empty() || root == nullptr || root->nextNodeVector->size() == 0)
    return;

  int index = 0;
  int lastMarkIndex = -1;
  auto currentNode = root;
  while (index < input.length()) {
    node *find = currentNode->findNextNode(input[index]);
    // 当没找到当前字符且failNode存在，切换到failNode继续查找
    while (find == nullptr && currentNode->failNode != nullptr) {
      currentNode = currentNode->failNode;
      find = currentNode->findNextNode(input[index]);
    }
    // 当failNode为空指针（此时currentNode一般为root）
    if (find == nullptr) {
      currentNode = root;
      index++;
      continue;
    }
    // 查找到字符
    currentNode = find;
    if (find->hasEnd) {
      // 最后标记
      //      int start = lastMarkIndex + 1 > index - find->wordNum + 1 ? lastMarkIndex + 1 : index - find->wordNum + 1;
      int start = index - find->wordNum + 1;
      //  for (int i = start; i <= index; i++)
      words.emplace_back(output.substr(start,find->wordNum));
      lastMarkIndex = index;
    }

    index++;
  }
}

double getSeconds(chrono::time_point<chrono::system_clock> &start,
                  chrono::time_point<chrono::system_clock> &end) {
  auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
  return double(duration.count()) / 1000000;
}

PYBIND11_MODULE(acfilter, m) {
  m.doc() = "sens word filter with AC auto machine"; // optional module docstring
  m.def("add", &add, "A function which adds two numbers");
}

int main() {
  auto start = chrono::system_clock::now(); // 开始时间
  SetConsoleOutputCP(CP_UTF8); // 设置window控制台编码为utf-8
  ifstream ifs; // 实例化文件读取类
  vector<string> word_list; // 定义链表
  // reserve的作用是更改vector的容量（capacity），使vector至少可以容纳n个元素。
  // 如果n大于vector当前的容量，reserve会对vector进行扩容。其他情况下都不会重新分配vector的存储空间
  word_list.reserve(200000);
  string word;
  cout << "start DFA" << endl;
  ifs.open("key.txt");
  if (!ifs)
    return 1;
  // 按行读取为字典链表 无大括号只执行最近的一条语句
  while (getline(ifs, word))
    word_list.emplace_back(word);

  ifs.close();
  cout << "load key file" << endl;
  auto end = chrono::system_clock::now();
  cout << "Create word_list num= " << word_list.size() << " time= " << getSeconds(start, end) << "s"
  << endl;

  string input;
  string output;
  vector<string> words2;
  ifs.open("input.txt");
  if (!ifs)
    return 1;
  cout << "load input file" << endl;

  // 按行读取要检测的文本
  while (getline(ifs, word))
    input += word;

  ifs.close();

  cout << "input: " << input << endl << endl;
  start = chrono::system_clock::now();
  // 暴力法敏感词过滤
  WordFilterNormal(input, word_list, output, words2);
  end = chrono::system_clock::now();
  cout << "Word Filter Normal time= " << getSeconds(start, end) << "s" << endl;
  //  cout << "output: " << output << endl << endl;
  for (const auto &c : words2) cout << c << " ";

  start = chrono::system_clock::now();
  auto WordTree = new node('\0');
  WordTree->nextNodeVector->reserve(256);
  CreateWordTree(word_list, WordTree);
  end = chrono::system_clock::now();
  cout << "Create Filtered Word Tree time= " << getSeconds(start, end) << "s" << endl;

  start = chrono::system_clock::now();
  CreateACAutomation(WordTree);
  end = chrono::system_clock::now();
  cout << "Create AC Automation time= " << getSeconds(start, end) << "s" << endl;

  start = chrono::system_clock::now();
  string output2;
  vector<string> words;
  WordFilterDFA(input, WordTree, output2, words);
  end = chrono::system_clock::now();
  cout << "Word Filter DFA time= " << getSeconds(start, end) << "s" << endl;
  cout << "word output: " << words[0] << endl;
  for (const auto &c : words) cout << c << " ";
  return 0;
}


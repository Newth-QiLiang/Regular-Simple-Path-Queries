#pragma once

#include<iostream>
#include<cstring>
#include<string>
#include<stack>
#include<vector>
#include<set>
#include<queue>
#include<unordered_map> 
#include<unordered_set> 

#define MAX 128

using namespace std;

typedef set<int> IntSet;
typedef set<char> CharSet;

struct edge{
    int toNode;
    int label;
};

struct NfaState				/*定义NFA状态*/
{
	int index;				/*NFA状态的状态号*/ 
	char input;				/*NFA状态弧上的值*/
	int chTrans;			/*NFA状态弧转移到的状态号*/ 
	IntSet epTrans;			/*当前状态通过ε转移到的状态号集合*/ 
};

struct NFA
{
	NfaState *head;			/*NFA的头指针*/
	NfaState *tail;			/*NFA的尾指针*/
};

struct Edge			/*定义DFA的转换弧*/
{
	char input;			/*弧上的值*/
	int Trans;			/*弧所指向的状态号*/
};

struct DfaState		/*定义DFA状态*/
{
	bool isEnd;			/*是否为终态，是为true，不是为false*/
	int index;			/*DFA状态的状态号*/
	IntSet closure;		/*NFA的ε-move()闭包*/	
	int edgeNum;		/*DFA状态上的射出弧数*/
	Edge Edges[10];		/*DFA状态上的射出弧*/
};

struct DFA			/*定义DFA结构*/
{
	int startState;				/*DFA的初态*/
	set<int> endStates;			/*DFA的终态集*/
	set<char> terminator;		/*DFA的终结符集*/
	int trans[MAX][26];		/*DFA的转移矩阵*/
};

struct stateSet			/*划分状态集*/
{
	int index;			/*该状态集所能转换到的状态集标号*/  
	IntSet s;			/*该状态集中的dfa状态号*/
};

struct FinalDFA
{
    int startState;
    set<int> endStates;
	int stateNum;
    set<int> labelSet;
    vector<unordered_map<int,int>> labelToState;//状态转移函数，[state][label]---->state;
	vector<unordered_map<int,unordered_set<int> > > Back_labelToState;//反向状态转移函数，[state][label]---->state set;
	unordered_map<int,unordered_set<int>> LabelArriveState;//label可以到达的状态，[label]----->state set;
};

class TreeNode{
public:
	bool start;
	int value;//node id
	int label;//edge label
	bool finish;//end or not
	unordered_map<int,unordered_map<int,TreeNode*> > nextnode;
	TreeNode(int _value){
		value = _value;
		finish = false;
		start = false;
	}
	TreeNode(int _value,int _label){
		value = _value;
		label = _label;
		finish = false;
		start = false;
	}
};
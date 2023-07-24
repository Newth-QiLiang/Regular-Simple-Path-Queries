#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "RegexToDFA.h"
#include "Struct_type.h"
#include "TimerClock.h"

using namespace std;

int numNode = 0;
int numEdge = 0;
int labelNum = 0;
int resultNumber = 0;
vector<string> regex_result;
FinalDFA Pre,Type,Suf;
TimerClock TC;
vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;
unordered_set <int> passnode_forward;
vector<int> path_forward;
vector<unordered_map<int,unordered_set<int> > > forward_path;
vector<vector<int>> forward_path_infor;
unordered_set <int> passnode_backward;
vector<int> path_backward;
vector<unordered_map<int,unordered_set<int> > > backward_path;
vector<vector<int>> backward_path_infor;
vector<int> type_path;
vector<string> final_result;
vector<string> type_result;
vector<vector<bool> > Block;
vector<vector<unordered_map<int,unordered_set<int> > > >  ConflictSet_type;//form : set[node][state] -> ([node][state])
vector<int> resultNode_type;
vector<int> resultLabel_type;
vector<unordered_map<int,bool> > BFS_Arrive_b;
queue<pair<int,int> > Q_F;
queue<pair<int,int> > Q_B;


vector<string> Split(const string& s, const char delim = '\t') {
	vector<string> elements;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
}

vector<string> RegexDivide(string str){
    int size = str.length();
    string pre;
    string type;
    string suff;
    vector<string> regex;
    int i = size - 1;
    bool s = true;
    bool p = false;
    bool t = false;
    while(i != 0){
        if(str[i] == '*'){
            t = true;
            s = false;
        }
        if(s){
            if(str[i] == '&' && str[i-1] == '*'){
                //
            }
            else{
                suff = str[i] + suff;
            }
        }
        if(t){
            if(str[i] == '&' && str[i-1] != '*'){
                p = true;
                t = false;
                i = i - 1;
            }else{
                type = str[i] + type;
            }
        }
        if(p){
            pre = str[i] + pre;
        }
        i--;
    }
    if(s){
        suff = str[i] + suff;
    }
    if(p){
        pre = str[i] + pre;
    }
    if(t){
        type = str[i] + type;
    }
    string str1 = "";
    string temp = "";
    for(int i = 0;i < suff.length();i++){
        if(suff[i] == '&'){
            str1 = suff[i] + temp + str1;
            temp = "";
        }else{
            temp = temp + suff[i];
        }
    }
    suff = temp + str1;
    regex.emplace_back(pre);
    regex.emplace_back(type);
    regex.emplace_back(suff);
    return regex;
}

void ReadData(string edgedata){
    ifstream infile(edgedata,ios::in);
    string line;
    getline(infile,line);
    istringstream ss(line);
    ss >> numNode;
    ss >> labelNum;
    graph.resize(numNode + 1);
    Back_graph.resize(numNode + 1);
    while(getline(infile, line)){
        istringstream ss(line);
        int src,dst,label;
        ss >> src;
        ss >> dst;
        ss >> label;
        if(src != dst){
            edge temp;
            temp.toNode = dst;
            temp.label = label;
            graph[src].push_back(temp);
            edge temp1;
            temp1.toNode = src;
            temp1.label = label;
            Back_graph[dst].push_back(temp1);
        }
    }
};

void DFS_forward(int node, int state){
    passnode_forward.insert(node);
    path_forward.push_back(node);
    for(auto it = graph[node].begin(); it != graph[node].end();it++){
        if(!Pre.labelToState[state].empty()){
            int nextnode = it->toNode;
            int label = it->label;
            if(Pre.labelToState[state].count(label)){
                int nextstate = Pre.labelToState[state][label];
                if(Pre.endStates.count(nextstate)){
                    unordered_map<int, unordered_set<int> > temp;
                    temp[nextnode] = passnode_forward;
                    forward_path.push_back(temp);
                    forward_path_infor.push_back(path_forward);
                }
                if(!passnode_forward.count(nextnode)){
                    DFS_forward(nextnode,nextstate);
                }
            }
        }
    }
    passnode_forward.erase(node);
    path_forward.pop_back();
}

void DFS_backward(int node, int state){
    passnode_backward.insert(node);
    path_backward.push_back(node);
    for(auto it = Back_graph[node].begin(); it != Back_graph[node].end(); it++){
        if(!Suf.labelToState[state].empty()){
            int nextnode = it->toNode;
            int label = it->label;
            if(Suf.labelToState[state].count(label)){
                int nextstate = Suf.labelToState[state][label];
                if(Suf.endStates.count(nextstate)){
                    unordered_map<int, unordered_set<int> > temp;
                    temp[nextnode] = passnode_backward;
                    backward_path.push_back(temp);
                    backward_path_infor.push_back(path_backward);
                }
                if(!passnode_backward.count(nextnode)){
                    DFS_backward(nextnode,nextstate);
                }
            }
        }
    }
    passnode_backward.erase(node);
    path_backward.pop_back();
}

void clear_vector(){
    Block.clear();
    ConflictSet_type.clear();
    resultNode_type.clear();
    resultLabel_type.clear();
    BFS_Arrive_b.clear();
    type_result.clear();
}

void initialize_vector(){
    resultNumber = 0;
    Block.resize(numNode);
    ConflictSet_type.resize(numNode);
    BFS_Arrive_b.resize(numNode);
}

void BFS_B(int dst,int end_state){
    pair<int,int> temp;
    temp.first = dst;
    temp.second = end_state;
    Q_B.push(temp);
    BFS_Arrive_b[dst][end_state] = true;
    while(!Q_B.empty()){
        int node = Q_B.front().first;
        int state = Q_B.front().second;
        Q_B.pop();
        for(auto it = Back_graph[node].begin();it != Back_graph[node].end();it++){
            int label = it->label;
            if(Type.Back_labelToState[state].count(label)){
                int nextnode = it->toNode;
                for(auto iter = Type.Back_labelToState[state][label].begin();iter != Type.Back_labelToState[state][label].end();iter++){
                    int nextstate = *iter;
                    if(!BFS_Arrive_b[nextnode].count(nextstate)){
                        temp.first = nextnode;
                        temp.second = nextstate;
                        Q_B.push(temp);
                        BFS_Arrive_b[nextnode][nextstate] = true;
                    }
                }
            }
        }
    }
}

void BFS(int src,int dst){
    for(auto it = Type.endStates.begin();it != Type.endStates.end();it++){
        BFS_B(dst,*it);
    }
}

bool IFblock(int u,int s,int u1,int s1){
    bool f = false;
    if(ConflictSet_type[u][s].count(u1)){
        if(ConflictSet_type[u][s][u1].count(s1)){
            return true;
        }
    }
    return f;
}

void unblock(int u,int s){
    Block[u][s] = false;
    for(auto it = ConflictSet_type[u][s].begin();it != ConflictSet_type[u][s].end();it++){
        int node = it->first;
        for(auto iter = it->second.begin();iter != it->second.end();iter++){
            int state = *iter;
            if(Block[node][state]){
                Block[node][state] = false;
                unblock(node,state);
            }
        }
    }
    ConflictSet_type[u][s].clear();
}

bool Type_path(int node,int dst,int state,unordered_set<int> forward_nodes, unordered_set<int> backward_nodes){
    bool f = false;
    // if(TC.getTimerSecond() > 600){
    //     return false;
    // }
    if(!BFS_Arrive_b[node][state]){
        Block[node][state] = true;
        return false;
    }
    for(int i = 0;i < Type.stateNum;i++){
        Block[node][state] = true;//block node+state;
    }

    resultNode_type.push_back(node);
    if(!Type.labelToState[state].empty()){
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            int label = it->label;
            if(Type.labelToState[state].count(label)){
                int nextnode = it->toNode;
                int nextstate = Type.labelToState[state][label];
                resultLabel_type.push_back(label);
                //forbid;
                if(forward_nodes.count(nextnode) || backward_nodes.count(nextnode)){
                    continue;
                }
                if(nextnode == dst && Type.endStates.count(state)){
                    resultNumber++;
                    string path = "";
                    for(int i = 0;i < resultNode_type.size();i++){
                        path = path + to_string(resultNode_type[i]) + "---<" + to_string(resultLabel_type[i]) + ">---";
                    }
                    path = path + to_string(dst);
                    type_result.push_back(path);
                    f = true;
                }
                else if(!Block[nextnode][nextstate]){
                    if(Type_path(nextnode,dst,nextstate,forward_nodes,backward_nodes)){
                        f = true;
                    }
                }
                resultLabel_type.pop_back();
            }
        }
    }
    resultNode_type.pop_back();
    if(f){
        //unblock v;
        for(int i = 0;i < Type.stateNum;i++){
            unblock(node,i);
        }
    }else{
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            int label = it->label;
            if(Type.labelToState[state].count(label)){
                int nextnode = it->toNode;
                int nextstate = Type.labelToState[state][label];
                if(!IFblock(nextnode,nextstate,node,state) && BFS_Arrive_b[node][state]){
                    ConflictSet_type[nextnode][nextstate][node].insert(state);
                }
            }
        }
    }
    return f;
}

void RTRE(int src,int dst,string regex,unordered_set<int> forward_nodes, unordered_set<int> backward_nodes){ 
    RegexToDFA d;
    d.update(regex);
    Type = d.dfa;
    for(int i = 0; i < ConflictSet_type.size();i++){
        ConflictSet_type[i].resize(Type.stateNum);
    }
    for(int i = 0; i < Block.size();i++){
        Block[i].resize(Type.stateNum);
    }
    BFS(src,dst);
    bool f = BFS_Arrive_b[src][Type.startState];
    if(f){
        Type_path(src,dst,Type.startState,forward_nodes,backward_nodes);
    }
}

void out_result(int i, int j){
    string forward = "";
    for(int k = 0; k < forward_path_infor[i].size(); k++){
        forward += to_string(forward_path_infor[i][k]);
    }
    string backward = "";
    for(int m = 0; m < backward_path_infor[j].size(); m++){
        backward += to_string(backward_path_infor[j][m]);
    }
    for(int n = 0; n < type_result.size(); n++){
        string final_string = "";
        final_string = forward + "---" + type_result[n] + "---" + backward;
        final_result.push_back(final_string);
        cout << final_string << endl;
    }
}

void RSPEQ(int src, int dst, string regex){
    regex_result = RegexDivide(regex);
    RegexToDFA d;
    d.update(regex_result[0]);
    Pre = d.dfa;
    RegexToDFA d2;
    d2.update(regex_result[2]);
    Suf = d2.dfa;
    int result = 0;
    TimerClock Simple_linked_time;
    Simple_linked_time.update();
    DFS_forward(src, Pre.startState);
    DFS_backward(dst, Suf.startState);
    cout << "link_time " << Simple_linked_time.getTimerMicroSec() << endl;
    for(int i = 0; i < forward_path.size(); i++){
        for(auto it = forward_path[i].begin(); it != forward_path[i].end(); it++){
            int src = it->first;
            unordered_set<int> node1 = it->second;
            for(int j = 0; j < backward_path.size(); j++){
                for(auto it1 = backward_path[j].begin(); it1 != backward_path[j].end(); it1++){
                    int dst = it1->first;
                    unordered_set<int> node2 = it1->second;
                    initialize_vector();
                    RTRE(src,dst,regex_result[1],node1,node2);
                    out_result(i,j);
                    clear_vector();
                }
            }
        }
    }
}

int main(int argc, char* argv[]){
    string edgeFile = argv[1];
    string queryFile = argv[2];
    string resultFile = argv[3];

    // string edgeFile = "/home/QiLiang/RSPQ/Text_data/graph.txt";
    // string queryFile = "/home/QiLiang/RSPQ/Text_data/query.txt";
    // string resultFile = "/home/QiLiang/RSPQ/Text_data/result.txt";

    ReadData(edgeFile);//read data;
    cout<<"node number: " << numNode << endl;
    cout<<"label number: " << labelNum << endl;

    ifstream query;
    query.open(queryFile.c_str());
    string line;
    vector<string> temp;
    int i = 0;
    ofstream s;
    s.open(resultFile.c_str(),ios::out);
    s.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
    s.precision(4);  // 设置精度 4
    while(getline(query,line)){
        temp = Split(line,' ');
        int src = stol(temp[0]);
        int dst = stol(temp[1]);
        string regex = temp[2];
        //cout << src << " " << dst << " " << regex << endl;
        i++;
        TC.update();
        RSPEQ(src,dst,regex);
        double time = TC.getTimerMicroSec();
        cout << "final time" << time << endl;
    }
    query.close();
    s.close();
    return 0;
}
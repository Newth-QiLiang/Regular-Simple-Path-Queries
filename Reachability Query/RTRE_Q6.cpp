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
vector<string> regex_result;
FinalDFA Pre,Type,Suf;
TimerClock TC;
TimerClock Simple_linked_time;
double simple_link_time;
vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;
vector<unordered_map<int,bool> > BFS_Arrive_f;
vector<unordered_map<int,bool> > BFS_Arrive_b;
queue<pair<int,int> > Q_F;
queue<pair<int,int> > Q_B;
unordered_set <int> passnode_forward;
vector<unordered_map<int,unordered_set<int> > > forward_path;
unordered_set <int> passnode_backward;
vector<unordered_map<int,unordered_set<int> > > backward_path;
//record edge information;
//vector<vector<edge>> edge_record;

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
                }
                if(!passnode_forward.count(nextnode)){
                    DFS_forward(nextnode,nextstate);
                }
            }
        }
    }
    passnode_forward.erase(node);
}

void DFS_backward(int node, int state){
    passnode_backward.insert(node);
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
                }
                if(!passnode_backward.count(nextnode)){
                    DFS_backward(nextnode,nextstate);
                }
            }
        }
    }
    passnode_backward.erase(node);
}

void init(){
    while(!Q_B.empty()){
        Q_B.pop();
    }
    while(!Q_F.empty()){
        Q_F.pop();
    }
    BFS_Arrive_b.clear();
    BFS_Arrive_f.clear();
    BFS_Arrive_b.resize(numNode);
    BFS_Arrive_f.resize(numNode);
}

int BBFS_BT(int src, int dst, unordered_set<int> forward_nodes, unordered_set<int> backward_nodes){
    init();
    Q_F.push(make_pair(src,Type.startState));
    BFS_Arrive_f[src][Type.startState] = true;
    for(auto it = Type.endStates.begin();it != Type.endStates.end();it++){
        Q_B.push(make_pair(dst,*it));
        BFS_Arrive_b[dst][*it] = true;
    }
    while(!Q_F.empty() && !Q_B.empty()){
        int node = Q_B.front().first;
        int state = Q_B.front().second;
        Q_B.pop();
        for(auto it = Back_graph[node].begin();it != Back_graph[node].end();it++){
            int label = it->label;
            if(Type.Back_labelToState[state].count(label)){
                int nextnode = it->toNode;
                //forbid;
                if(forward_nodes.count(nextnode) || backward_nodes.count(nextnode)){
                    continue;
                }
                for(auto iter = Type.Back_labelToState[state][label].begin();iter != Type.Back_labelToState[state][label].end();iter++){
                    int nextstate = *iter;
                    if(!BFS_Arrive_b[nextnode].count(nextstate)){
                        Q_B.push(make_pair(nextnode,nextstate));
                        BFS_Arrive_b[nextnode][nextstate] = true;
                    }
                    if(BFS_Arrive_f[nextnode].count(nextstate)){
                        return 1;
                    }
                }
            }
        }
        node = Q_F.front().first;
        state = Q_F.front().second;
        Q_F.pop();
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            int label = it->label;
            if(Type.labelToState[state].count(label)){
                int nextnode = it->toNode;
                //forbid;
                if(forward_nodes.count(nextnode) || backward_nodes.count(nextnode)){
                    continue;
                }
                int nextstate = Type.labelToState[state][label];
                if(!BFS_Arrive_f[nextnode].count(nextstate)){
                    Q_F.push(make_pair(nextnode,nextstate));
                    BFS_Arrive_f[nextnode][nextstate] = true;
                }
                if(BFS_Arrive_b[nextnode].count(nextstate)){
                    return 1;
                }
            }
        }
    }
    return 0;
}

int RSPRQ(int src, int dst, string regex){
    regex_result = RegexDivide(regex);
    RegexToDFA d;
    d.update(regex_result[0]);
    Pre = d.dfa;
    RegexToDFA d1;
    d1.update(regex_result[1]);
    Type = d1.dfa;
    RegexToDFA d2;
    d2.update(regex_result[2]);
    Suf = d2.dfa;
    int result = 0;
    Simple_linked_time.update();
    DFS_forward(src, Pre.startState);
    DFS_backward(dst, Suf.startState);
    simple_link_time = Simple_linked_time.getTimerMicroSec();
    for(int i = 0; i < forward_path.size(); i++){
        for(auto it = forward_path[i].begin(); it != forward_path[i].end(); it++){
            int src = it->first;
            unordered_set<int> node1 = it->second;
            for(int j = 0; j < backward_path.size(); j++){
                for(auto it1 = backward_path[j].begin(); it1 != backward_path[j].end(); it1++){
                    int dst = it1->first;
                    unordered_set<int> node2 = it1->second;
                    result = BBFS_BT(src,dst,node1,node2);
                    if(result == 1){
                        return result;
                    }
                }
            }
        }
    }
    return result;
}

int main(int argc, char* argv[]){

    string edgeFile = argv[1];
    string queryFile = argv[2];
    string resultFile = argv[3];

    // string edgeFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/AD/advogato";
    // string queryFile = "/home/liangqi/Projects/QiLiang/RSPQ/Code/Data/AD/advogato_reach.query";
    // string resultFile = "/home/liangqi/Projects/QiLiang/RSPQ/Code/Data/AD/advogato_reach.result";

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
        init();
        TC.update();
        int result = RSPRQ(src,dst,regex);
        double time = TC.getTimerMicroSec();
        s <<  i << " " << result << " " << simple_link_time << " " << time  << endl;
    }
    query.close();
    s.close();

    // vector<string> regex_result;
    // string regex = "1&2&3&(0)*&(1)*&2&3&4";
    // regex_result = RegexDivide(regex);
    // for(int i = 0; i < 3; i++){
    //     cout << regex_result[i] << endl;
    // }
    return 0;
}
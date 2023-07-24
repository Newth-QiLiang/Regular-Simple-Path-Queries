#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include "RegexToDFA.h"
#include "TimerClock.h"


using namespace std;

int numNode = 0;
int numEdge = 0;
int labelNum = 0;
int resultNumber = 0;
vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;
unordered_map<long long, int> v_index1;
unordered_map<int, long long> v_index2;//node mapping;
unordered_map<int, int> labelIndex1;
unordered_map<int, int> labelIndex2;//label mapping;
vector<unordered_map<int,unordered_set<int> > > conflictSet;//form : set[node][state] -> set
vector<unordered_map<int,unordered_set<int> > > conflictSetUnion;//form : set[node][state] -> set
vector<int> resultsNode,resultsLabel,resultState;
vector<string> result;
unordered_set<int> passNode;
FinalDFA dfa; 
TimerClock TC;
TimerClock Bfs;
//bfs:
vector<unordered_map<int,bool> > BFS_Arrive;
queue<pair<int,int> > Q;


vector<string> Split(const string& s, const char delim = '\t') {
	vector<string> elements;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
}

bool pathConflict(int node, int state){
    for(auto it = conflictSet[node][state].begin();it != conflictSet[node][state].end();it++){
        if(passNode.count(*it)){
            return true;
        }
    }
    return false;
}

void update_conflictSet(int node,int state){
    //type:父节点的冲突集合为子节点冲突集合、探索过程中自己的冲突集合的交集;
    //note:忽略不可达的子节点;
    int number = 0;
    vector<int> setTemp;
    unordered_set<int> temp;
    for(auto it = graph[node].begin();it != graph[node].end();it++){
        int nextnode = it->toNode;
        if(dfa.labelToState[state].count(it->label)){
            int nextstate = dfa.labelToState[state][it->label];
            if(!BFS_Arrive[nextnode][nextstate]){
                if(conflictSetUnion[node][state].count(nextnode)){
                    setTemp.push_back(nextnode);
                    number++;
                }else{
                    number++;
                    for(auto it1 = conflictSet[nextnode][nextstate].begin();it1 != conflictSet[nextnode][nextstate].end();it1++){
                        setTemp.push_back(*it1);
                    }
                }
            }
        }
    }
    if(number == 1){
        for(auto it = setTemp.begin();it != setTemp.end();it++){
            conflictSet[node][state].insert(*it);
        }
    }else{
        for(auto it = setTemp.begin();it != setTemp.end();it++){
            if(temp.count(*it)){
                conflictSet[node][state].insert(*it);
            }else{
                temp.insert(*it);
            }
        }
    }
    setTemp.clear();
    temp.clear();
}

void BFS(int endID,int end_state){
    pair<int,int> temp;
    temp.first = endID;
    temp.second = end_state;
    Q.push(temp);
    BFS_Arrive[endID][end_state] = true;
    while(!Q.empty()){
        int node = Q.front().first;
        int state = Q.front().second;
        Q.pop();
        for(auto it = Back_graph[node].begin();it != Back_graph[node].end();it++){
            int label = it->label;
            if(dfa.Back_labelToState[state].count(label)){
                int nextnode = it->toNode;
                for(auto iter = dfa.Back_labelToState[state][label].begin();iter != dfa.Back_labelToState[state][label].end();iter++){
                    int nextstate = *iter;
                    if(!BFS_Arrive[nextnode].count(nextstate)){
                        temp.first = nextnode;
                        temp.second = nextstate;
                        Q.push(temp);
                        BFS_Arrive[nextnode][nextstate] = true;
                    }
                }
            }
        }
    }
}

bool pruning(int node,int state,int dst){
    bool f = false;
    if(TC.getTimerSecond() > 600){
        return false;
    }
    if(!BFS_Arrive[node][state]){
        return false;
    }
    passNode.insert(node);
    if(pathConflict(node,state)){
        //exist conflict and no exist path to dst node, stop exploring; otherwise, go on exploring.
    }else{
        if(!dfa.labelToState[state].empty()){
            for(auto it = graph[node].begin();it != graph[node].end();it++){
                resultsNode.push_back(node);
                resultState.push_back(state);
                if(dfa.labelToState[state].count(it->label)){
                    int nextnode = it->toNode;
                    int nextstate = dfa.labelToState[state][it->label];
                    resultsLabel.push_back(it->label);
                    if(nextnode == dst && dfa.endStates.count(nextstate)){
                        //find a path from src to dst;
                        //cout << "find path!"<<endl;
                        f = true;
                        resultNumber++;
                        // int i;
                        // string path = "";
                        // for(i = 0; i < resultsNode.size() - 1; ++i){
                        //     path = path + to_string(resultsNode[i]) + "---<" + to_string(resultsLabel[i]) + ">---";
                        // }
                        // //记录路径信息,可以改进的一个地方--即每次判断是否存在花费大量时间；可以改进的一个地方,结果的存储（去除冗余）;
                        // path = path + to_string(resultsNode[i]) + "---<" + to_string(resultsLabel[i]) + ">---" + to_string(dst);
                        // result.push_back(path);
                    }
                    if(nextnode != dst && !passNode.count(nextnode)){
                        //not destination and up to the simple rule;
                        //cout << "node : " << node << "  next node : " << nextnode  << endl;
                        pruning(nextnode,nextstate,dst);
                    }
                    if(nextnode != dst && passNode.count(nextnode)){
                        conflictSetUnion[node][state].insert(nextnode);
                    }         
                    resultsLabel.pop_back();
                }
                resultsNode.pop_back();
                resultState.pop_back();
            }
            update_conflictSet(node,state);//更新节点冲突集合；
        }      
    }
    passNode.erase(node);
    return f;
}

double DFS_pruning(int src,int dst,string regex){
    RegexToDFA d;
    d.update(regex);
    dfa = d.dfa;
    double time;
    Bfs.update();
    for(auto it = dfa.endStates.begin();it != dfa.endStates.end();it++){
        BFS(dst,*it);
    }
    time = Bfs.getTimerMilliSec();
    //cout << time << endl;
    if(BFS_Arrive[src][dfa.startState]){
        pruning(src,dfa.startState,dst);
    }
    return time;
}

void outputResult(string outfile,int i){
    ofstream s;
    s.open(outfile.c_str(),ios::app);
    s <<  i << "th query:" << endl;
    for(int i = 0; i < result.size(); i++){
        s << result[i] << endl;
    }
    s.close();
}

void ResultTime(string outfile, double time,int i,double bfs_time){
    ofstream s;
    s.open(outfile.c_str(),ios::app);
    s.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
    s.precision(4);  // 设置精度 4
    s <<  i << " " << resultNumber << " " << time  << " " << bfs_time << endl;  //form : queryNumber,result number, time(s);
}

void clear_vector(){
    resultsNode.clear();
    resultsLabel.clear();
    passNode.clear();
    result.clear();
    conflictSet.clear();
    conflictSetUnion.clear();
    resultState.clear();
    BFS_Arrive.clear();
}

void initialize_vector(){
    resultNumber = 0;
    conflictSet.resize(numNode);
    conflictSetUnion.resize(numNode);
    BFS_Arrive.resize(numNode);
}

void ReadData(string edgedata){
    //edgedata form : start, end, label;
    FILE* edgeFile;
    edgeFile = fopen(edgedata.c_str(),"r");
    vector<edge> temp;
    unordered_set<int> temp1;
    while(1){
        int ret;
        long long src,dst,label;
        ret = fscanf(edgeFile,"%lld %lld %lld",&src,&dst,&label);
        if(ret == EOF){
            break;
        }
        if(!v_index1.count(src)){
            v_index1[src] = numNode;
            v_index2[numNode] = src;
            numNode++;
            graph.push_back(temp);
            Back_graph.push_back(temp);
        }
        if(!v_index1.count(dst)){
            v_index1[dst] = numNode;
            v_index2[numNode] = dst;
            numNode++;
            graph.push_back(temp);
            Back_graph.push_back(temp);
        }
        if(!labelIndex1.count(label)){
            labelIndex1[label] = labelNum;
            labelIndex2[labelNum] = label;
            labelNum++;
        }
        if(src != dst){
            //forward graph;
            edge temp2;
            temp2.toNode = v_index1[dst];
            temp2.label = labelIndex1[label];
            graph[v_index1[src]].push_back(temp2);
            //backward graph;
            edge back_temp;
            back_temp.toNode = v_index1[src];
            back_temp.label = labelIndex1[label];
            Back_graph[v_index1[dst]].push_back(back_temp);
            numEdge++;
        }
    }//save every edge in the graph;
};

int main(int argc, char* argv[]){

    string edgeFile = argv[1];
    string queryFile = argv[2];
    string resultFile = argv[3];
    string TimeFile = argv[4];

    ReadData(edgeFile);//read data;
    cout<<"finishing reading data"<< endl;
    // cout<<"node number: " << numNode << endl;
    // cout<<"edge number: " << numEdge << endl;
    // cout<<"label number: " << labelNum << endl;

    ifstream query;
    //query form : start end regex;
    query.open(queryFile.c_str());
    string line;
    vector<string> temp;
    int i = 0;
    while(getline(query,line)){
        temp = Split(line,' ');
        long long src = stoll(temp[0]);
        long long dst = stoll(temp[1]);
        string regex = temp[2];
        i++;
        //cout << src << " " << dst << " " << regex << endl;
        initialize_vector();
        TC.update();
        double bfs_time = DFS_pruning(src,dst,regex);
        double time = TC.getTimerMilliSec();
        //outputResult(resultFile,i);
        ResultTime(TimeFile,time,i,bfs_time);
        clear_vector();//清除数组内容以及释放空间
    }
    query.close();
    return 0;
}
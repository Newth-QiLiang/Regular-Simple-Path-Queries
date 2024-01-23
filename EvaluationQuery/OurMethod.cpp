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

FinalDFA dfa;
TimerClock TC;
int numNode = 0;
int numEdge = 0;
int labelNum = 0;
int resultNumber = 0;
vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;

vector<unordered_map<int,bool> > BFS_Arrive_f;
vector<unordered_map<int,bool> > BFS_Arrive_b;
queue<pair<int,int> > Q_F;
queue<pair<int,int> > Q_B;

unordered_set<int> passNode;

vector<unordered_set<int> > QR;
vector<pair<int,int> > node_pair;

vector<edge> recover_graph;

vector<string> Split(const string& s, const char delim = '\t') {
	vector<string> elements;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
}

void init(){
    while(!Q_F.empty()){
        Q_F.pop();
    }
    BFS_Arrive_f.clear();

    QR.clear();
    QR.resize(numNode);
    node_pair.clear();
    resultNumber = 0;
}

void clear(){
    BFS_Arrive_f.clear();
    BFS_Arrive_f.resize(numNode);
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

void DeleteGraph(int src){
    recover_graph = graph[src];
    graph[src].clear();
}

void recover(int src){
    graph[src] = recover_graph;
    recover_graph.clear();
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
}

void bfs(int source, unordered_set<int> node_set, int start_state){
    if(TC.getTimerSecond() > 3600){
        return;
    }
    for(auto it = node_set.begin(); it != node_set.end(); it++){
        pair<int, int> temp;
        temp.first = *it;
        temp.second = start_state;
        Q_F.push(temp);
        BFS_Arrive_f[*it][start_state] = true;
    }

    while(!Q_F.empty()){
        int node = Q_F.front().first;
        int state = Q_F.front().second;
        Q_F.pop();
        for(auto it = graph[node].begin(); it != graph[node].end(); it++){
            int label = it->label;
            if(dfa.labelToState[state].count(label)){
                int nextnode = it->toNode;
                int nextstate = dfa.labelToState[state][label];
                if(dfa.endStates.count(nextstate)){
                    QR[source].insert(nextnode);
                }
                if(!BFS_Arrive_f[nextnode].count(nextstate)){
                    Q_F.push(make_pair(nextnode,nextstate));
                    BFS_Arrive_f[nextnode][nextstate] = true;
                }
            }
        }
    }
}

void bfs_prunning(int source, int src, int start_state){
    if(TC.getTimerSecond() > 3600){
        return;
    }
    pair<int, int> temp;
    temp.first = src;
    temp.second = start_state;
    Q_F.push(temp);
    BFS_Arrive_f[src][start_state] = true;

    while(!Q_F.empty()){
        int node = Q_F.front().first;
        int state = Q_F.front().second;
        Q_F.pop();
        if(dfa.endStates.count(state)){
            QR[source].insert(node);
        }
        for(auto it = graph[node].begin(); it != graph[node].end(); it++){
            int label = it->label;
            if(dfa.labelToState[state].count(label)){
                int nextnode = it->toNode;
                int nextstate = dfa.labelToState[state][label];

                if(nextstate == dfa.startState && !QR[nextnode].empty()){
                    // QR[source].insert(QR[nextnode].begin(),QR[nextnode].end());
                    continue;
                }

                if(!BFS_Arrive_f[nextnode].count(nextstate)){
                    Q_F.push(make_pair(nextnode,nextstate));
                    BFS_Arrive_f[nextnode][nextstate] = true;
                }
            }
        }
    }
}

void dfs(int source, int node, int state){
    if(TC.getTimerSecond() > 3600){
        return;
    }
    passNode.insert(node);
    if(dfa.endStates.count(state)){
        QR[source].insert(node);
    }

    if(!dfa.labelToState[state].empty()){
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            if(dfa.labelToState[state].count(it->label)){
                int nextnode = it->toNode;
                int nextstate = dfa.labelToState[state][it->label];
                if(!passNode.count(nextnode)){
                    //cout << "node : " << node  << "  next node: " << nextnode << endl; 
                    dfs(source,nextnode,nextstate);
                }
            }
        }
    }
    passNode.erase(node); 
}

void random_query(int source, int node, int depth, int label_number, vector<int> label_set){
    if(depth < label_number){
        DeleteGraph(node);
        for(auto x : graph[node]){
            if(x.toNode == label_set[depth]){
                random_query(source, x.toNode, depth + 1, label_number, label_set);
            }
        }
        recover(node);
    }else if (depth == label_number){
        unordered_set<int> node_set;
        int label = label_set[label_number];
        for(auto x : graph[node]){
            if(x.label == label){
                node_set.insert(x.toNode);
            }
        }
        if(!node_set.empty()){
            clear();
            bfs(source, node_set, dfa.startState);
        }
    }
}


void RTRE(string regex){
    vector<string> regex_result = RegexDivide(regex);
    // for(int i = 0; i < regex_result.size(); i++){
    //     cout << i << ": " << regex_result[i] << endl;
    // }

    if(regex_result[0].empty() && regex_result[2].empty()){
        // cout << "type 1: " << regex << endl;
        TC.update();
        RegexToDFA d;
        d.update(regex);
        dfa = d.dfa;
        for(int src = 0; src < numNode; src++){
            clear();
            bfs_prunning(src,src,dfa.startState);
            // ReachSearch(src,src,regex);
        }
    }else if(regex_result[1].empty()){
        // cout << "type 2: " << regex << endl;
        TC.update();
        RegexToDFA d;
        d.update(regex);
        dfa = d.dfa;
        for(int src = 0; src < numNode; src++){
            dfs(src,src,dfa.startState);
            // ReachSearch(src,src,regex);
        }
    }else if(!regex_result[1].empty() && !regex_result[2].empty()){
        TC.update();
        RegexToDFA d;
        d.update(regex);
        dfa = d.dfa;
        for(int src = 0; src < numNode; src++){
            dfs(src,src,dfa.startState);
            // ReachSearch(src,src,regex);
        }
    }
    else{

        string regex_pre = "";
        for(int i = 0; i < regex_result[0].size(); i++){
            regex_pre += regex_result[0][i];
        }
        string type_regex = regex_result[1];

        // cout << "type 3: " << regex_pre << " " << type_regex << endl;

        TC.update();
        RegexToDFA d;
        d.update(type_regex);
        dfa = d.dfa;

        vector<unordered_map<int, unordered_set<int>>> node_set;
        vector<int> label_set;
        string label = "";
        for(int i = 0; i < regex_pre.size(); i++){
            if(regex_pre[i] != '&'){
                label += regex_pre[i];
            }else{
                label_set.push_back(stoll(label));
                label = "";
            }
        }

        for(int src = 0; src < numNode; src++){
        
            // int label = stol(regex_pre);
            // for(auto it = graph[src].begin(); it != graph[src].end();it++){
            //     if(it->label == label){
            //         node_set.insert(it->toNode);
            //     }
            // }

            // if(!node_set.empty()){
            //     DeleteGraph(src);
            //     // for(auto it = node_set.begin(); it != node_set.end(); it++){
            //     //     clear();
            //     //     bfs(src, *it, dfa.startState);
            //     //     // ReachSearch(src,*it,type_regex);
            //     // }
            //     clear();
            //     bfs(src, node_set, dfa.startState);
            //     recover(src);
            // }

            random_query(src, src, 0, label_set.size() - 1, label_set);
        }
    }

    for(int j = 0; j < numNode; j++){
        for(auto it = QR[j].begin();it != QR[j].end();it++){
            if(*it != j){
                node_pair.push_back({j,*it});
                resultNumber++;
            }
        }
    }
}

void RunBBFS(string queryfile, string resultfile){
    ifstream query;
    query.open(queryfile.c_str());
    string line;
    vector<string> temp;
    int i = 0;
    ofstream s;
    s.open(resultfile.c_str(),ios::out);

    while(getline(query,line)){
        string regex = line;
        // cout << regex << endl;
        i++;
        init();
        RTRE(regex);
        double time = TC.getTimerSecond();
        s <<  i << " " << resultNumber << " " << time << endl;
    }
    query.close();
    s.close();
}


int main(int argc, char* argv[]){

    string edgeFile = argv[1];
    // string edgeFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/AD/advogato";
    
    string queryFile = argv[2];
    string resultFile = argv[3];
    //string memoryfile = argv[4];

    ReadData(edgeFile);//read data;
    cout<<"finishing reading data"<< endl;
    cout<<"node number: " << numNode << endl;
    cout<<"edge number: " << numEdge << endl;
    cout<<"label number: " << labelNum << endl;

    // for(int type = 5; type < 7; type++){
    //     string queryFile = edgeFile + "_evaluation_Q" + to_string(type) + ".query";
    //     string resultFile = edgeFile + "_evaluation_Q" + to_string(type) + "_OurMethod.time.result";
    //     RunBBFS(queryFile,resultFile);
    // }

    RunBBFS(queryFile,resultFile);

    return 0;
}

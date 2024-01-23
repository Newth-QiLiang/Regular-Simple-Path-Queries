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
#include <ctime>
#include <time.h>
#include <random>
#include "RegexToDFA.h"
#include "Struct_type.h"
#include "TimerClock.h"

using namespace std;

int numNode = 0; 
int numEdge = 0;
int labelNum = 0;
int resultNumber = 0;
FinalDFA dfa;
TimerClock TC;

int result_number_temp = 0;
int depth_temp = 0; 

vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;
vector<vector<bool> > suffix_contain;
vector<vector<vector< pair<int,int> > > > list;

vector<vector<int> > cm;
vector<vector<int> > pm; 

// vector<unordered_set<int> > cm;
// vector<unordered_set<int> > pm; 

vector<unordered_set<int> > QR;
vector<pair<int,int> > node_pair;

vector<string> Split(const string& s, const char delim = '\t') {
	vector<string> elements;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
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

bool check_temp(int s, int t){
    for(auto it = dfa.labelSet.begin();it != dfa.labelSet.end();it++){
        int label = *it;
        int s1 = -1;
        int t1 = -1;
        if(dfa.labelToState[s].count(label)){
            s1 = dfa.labelToState[s][label];
        }
        if(dfa.labelToState[t].count(label)){
            t1 = dfa.labelToState[t][label];
        }
        if(t1 != -1 && s1 == -1){
            return true;
        }else if(t1 == -1){
            continue;
        }else{
            if(!suffix_contain[s1][t1]){
                return true;
            }    
        }
    }
    return false;
}

void mark(int s, int t){
    suffix_contain[s][t] = false;
    for(auto it = list[s][t].begin();it != list[s][t].end();it++){
        int s1 = it->first;
        int t1 = it->second;
        if(suffix_contain[s1][t1]){
            mark(s1,t1);
        }
    }
}

void check_suffix_contain(){
    int num = dfa.stateNum;
    suffix_contain.resize(num);
    for(int i = 0; i < num;i++){
        suffix_contain[i].resize(num,true);
    }
    list.resize(num);
    for(int i = 0; i < num;i++){
        list[i].resize(num);
    }
    for(int i  = 0; i < num;i++){
        if(!dfa.endStates.count(i)){
            for(auto it = dfa.endStates.begin(); it != dfa.endStates.end();it++){
                suffix_contain[i][*it] = false;
            }
        }
    }
    for(int i = 0; i < num; i++){
        for(int j = 0; j < num; j++){
            if(i == j){
                continue;
            }else if(!suffix_contain[i][j]){
                continue;
            }else{
                if(check_temp(i,j)){
                    mark(i,j);
                }else{
                    for(auto it = dfa.labelSet.begin();it != dfa.labelSet.end();it++){
                        int label = *it;
                        int s1,t1;
                        if(dfa.labelToState[i].count(label)){
                            s1 = dfa.labelToState[i][label];
                        }else{
                            continue;
                        }
                        if(dfa.labelToState[j].count(label)){
                            t1 = dfa.labelToState[j][label];
                        }else{
                            continue;
                        }
                        if(s1 != t1){
                            list[s1][t1].push_back({i,j});
                        }
                    }
                }
            }
        }
    }
}

int first(int w){
    if(!cm[w].empty()){
        return *(cm[w].begin());
    }else{
        return -1;
    }
}

bool cm_contain(int w, int t){
    for(auto it = cm[w].begin(); it != cm[w].end(); it++){
        if(*it == t){
            return true;
        }
    }
    return false;
}

bool pm_contain(int w, int t){
    for(auto it = pm[w].begin(); it != pm[w].end(); it++){
        if(*it == t){
            return true;
        }
    }
    return false;
}

bool search(int u,int v,int s,bool conflict){
    // cout << "search node: " << v << " depth: " << depth_temp << endl;
    conflict = false;
    cm[v].push_back(s);

    if(TC.getTimerSecond() > 3600){
        return false;
    }

    depth_temp++;
    if(dfa.endStates.count(s)){
        // result_number_temp++;
        QR[u].insert(v);
        // cout << "result_number: " << QR[u].size() << endl;
    }

    for(auto it = graph[v].begin();it != graph[v].end();it++){
        int w = it->toNode;
        int a = it->label;
        int t;
        if(dfa.labelToState[s].count(a)){
            t = dfa.labelToState[s][a];
            // if(!cm[w].count(t) && !pm[w].count(t)){
            if(!cm_contain(w,t) && !pm_contain(w,t)){   
                // if(!cm[w].empty() && !suffix_contain[*(cm[w].begin())][t]){
                if(first(w) != -1 && !suffix_contain[first(w)][t]){
                    conflict = true;
                }else{
                    bool new_conflict;
                    new_conflict = search(u,w,t,new_conflict);
                    if(new_conflict || conflict){
                        conflict = true;
                    }else{
                        conflict = false;
                    }
                }
            }
        }
    }
    // cm[v].erase(s);
    cm[v].pop_back();

    depth_temp--;
    if(!conflict){
        pm[v].push_back(s);
    }
    return conflict;
}

void init(){
    resultNumber = 0;
    cm.clear();
    pm.clear();
    QR.clear();
    suffix_contain.clear();
    list.clear();
    node_pair.clear();
    cm.resize(numNode);
    pm.resize(numNode);
    QR.resize(numNode);
}

void wood(string str){
    RegexToDFA a;
    a.update(str);
    dfa = a.dfa;
    check_suffix_contain();
    bool conflict;
    for(int src = 0; src < numNode; src++){
        result_number_temp = 0;
        depth_temp = 0; 
        search(src,src,dfa.startState,conflict);
        for(int j = 0; j < numNode; j++){
            if(!pm[j].empty()){
                pm[j].clear();
            }
        }
        for(int j = 0; j < numNode; j++){
            if(!cm[j].empty()){
                cm[j].clear();
            }
        }
    }

    // output;
    for(int j = 0; j < numNode; j++){
        for(auto it = QR[j].begin();it != QR[j].end();it++){
            if(*it != j){
                node_pair.push_back({j,*it});
                resultNumber++;
            }
        }
    }
}

int main(int argc, char* argv[]){

    // string edgefile = argv[1];

    string edgefile = argv[1];
    ReadData(edgefile);//read data;

    cout << "finish read graph!" << endl;

    string queryFile = argv[2];
    string resultFile = argv[3];

    // for(int type = 5; type < 7; type++){
    //     string queryFile = edgefile + "_evaluation_Q" + to_string(type) + ".query";
    //     string resultFile = edgefile + "_evaluation_Q" + to_string(type) + "_Wood.time.result";

    //     ofstream s;
    //     s.open(resultFile.c_str(),ios::out);

    //     ifstream query;
    //     query.open(queryFile.c_str());

    //     string line;
    //     vector<string> temp;
    //     int i = 0;//queryNumber;
    //     while(getline(query,line)){
    //         temp = Split(line,' ');
    //         string regex = temp[0];
    //         i++;
    //         init();
    //         TC.update();
    //         wood(regex);
    //         double time = TC.getTimerSecond();
    //         s << i << " " << resultNumber << " " << time << endl;
    //     }
    //     query.close();
    //     s.close();
    // }

    ofstream s;
    s.open(resultFile.c_str(),ios::out);

    ifstream query;
    query.open(queryFile.c_str());

    string line;
    vector<string> temp;
    int i = 0;//queryNumber;
    while(getline(query,line)){
        temp = Split(line,' ');
        string regex = temp[0];
        i++;
        init();
        TC.update();
        wood(regex);
        double time = TC.getTimerSecond();
        s << i << " " << resultNumber << " " << time << endl;
    }
    query.close();
    s.close();
    return 0;
}
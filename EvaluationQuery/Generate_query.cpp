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

using namespace std;

int numNode = 0; 
int numEdge = 0;
int labelNum = 0;

vector<unordered_set<string>> query(7);

void ReadData(string edgedata){
    ifstream infile(edgedata,ios::in);
    string line;
    getline(infile,line);
    istringstream ss(line);
    ss >> numNode;
    ss >> labelNum;
    cout << numNode << " " << labelNum << endl;
};

void make_query(string graph){

    // string arr_query = graph + "_Type_arr.query";
    // string bbfs_query = graph + "_Type_bbfs.query";
        while (query[1].size() < 3){
            string str = "";
            int label = rand()%(labelNum);
            str = "(" + to_string(label) + ")*";
            query[1].insert(str);   
        }

        while (query[2].size() < 5){
            string str = "";
            int label = rand()%(labelNum);
            int pre = rand()%(labelNum);
            str = "(" + to_string(pre) + ")&(" + to_string(label) + ")*";
            query[2].insert(str);
        }

        while (query[3].size() < 5){
            string str = "";
            int label = rand()%(labelNum);
            int label_1 = rand()%(labelNum);
            int pre = rand()%(labelNum);
            str = "(" + to_string(pre) + ")&" + "(" + to_string(label) + ")*&" + "(" + to_string(label_1) + ")*";
            query[3].insert(str);
        }
    
        while (query[4].size() < 5){
            int query_size;
            if(labelNum < 6){
                query_size = 2;
            }else{
                query_size = rand()%(5)+2;
            }
            unordered_set<int> label_set;

            while(label_set.size() < query_size){
                int label = rand()%(labelNum);
                label_set.insert(label);
            }
            string str = "(";
            while (!label_set.empty()){
                auto it = label_set.begin();
                int label = *it;
                label_set.erase(label);
                if(label_set.empty()){
                    str += to_string(label) + ")*";
                }else{
                    str += to_string(label) + "|"; 
                } 
            }
            query[4].insert(str);
        }

        while (query[5].size() < 5){
            int query_size;
            if(labelNum < 6){
                query_size = 2;
            }else{
                query_size = rand()%(6)+2;
            }
            unordered_set<int> label_set;

            while(label_set.size() < query_size){
                int label = rand()%(labelNum);
                label_set.insert(label);
            }

            string str = "";
            while (!label_set.empty()){
                auto it = label_set.begin();
                int label = *it;
                label_set.erase(label);
                if(label_set.empty()){
                    str += to_string(label) + "";
                }else{
                    str += ""+ to_string(label) + "&"; 
                } 
            }
            query[5].insert(str);
        }

        while (query[6].size() < 5){
            int query_size;
            if(labelNum < 6){
                query_size = 2;
            }else{
                query_size = rand()%(6)+2;
            }
            unordered_set<int> label_set;

            while(label_set.size() < query_size){
                int label = rand()%(labelNum);
                label_set.insert(label);
            }

            string str = "(";
            while (!label_set.empty()){
                auto it = label_set.begin();
                int label = *it;
                label_set.erase(label);
                if(label_set.empty()){
                    str += to_string(label) + ")*";
                }else{
                    str += to_string(label) + ")*&("; 
                } 
            }
            int size = query[5].size();
            int number = rand()%(size);
            auto it = query[5].begin();
            for(int i = 0; i < number; i++){
                it++;
            }
            int choose = rand()%(4);
            if(choose == 0){
                str = str;
            }else if(choose == 1){
                str = *it + "&" + str;
            }else if(choose == 2){
                str = str + *it;
            }else{
                str = *it + "&" + str + *it;
            }
            query[6].insert(str);
        }

    for(int i = 1; i < query.size(); i++){
        string query_file = graph + "_evaluation_Q" + to_string(i) + ".query";
        ofstream evaluate(query_file,ios::out);
        for (auto it = query[i].begin(); it != query[i].end(); it++){
            evaluate << *it << endl;
        } 
        evaluate.close();
    }
}

int main(int argc, char* argv[]){

    string graph = argv[1];

    // string graph = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/BG/Biogrid";

    srand((unsigned int)time(NULL));
    ReadData(graph);//read data;

    make_query(graph);

    return 0;
}
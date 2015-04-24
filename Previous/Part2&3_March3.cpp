#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <string.h>
#include <sstream>
#include <vector>

#define IM_module_number	4	//also as 'M', and the number of MEMS port
#define IM_port_number		4	//also as 'N', and the number of MEMS module
#define ToR_number		16	//also as 'MN'
#define wavelength		10	//the number of wavelength in one fiber

using namespace std;
//--------------------------------------------------------------------------------------------------------------
//construct n multiply of m times m 2D matrix
//N		   M		     M
int MEMS_connection[IM_port_number][IM_module_number][IM_module_number];
int ToR_wavelength[ToR_number][ToR_number];
int overall_MEMS[IM_module_number][IM_module_number];
//--------------------------------------------------------------------------------------------------------------
//here to define the random number generator
static default_random_engine e(0);
static uniform_int_distribution<unsigned> u(0,IM_port_number*IM_port_number-1);
auto random_number=bind(u,e);
//--------------------------------------------------------------------------------------------------------------
void intialize() {
    for(auto &x: MEMS_connection) {
        for(auto &y: x) {
            for(auto &z: y)
                z=0;
        }
    }
    for(auto &x: ToR_wavelength) {
        for(auto &y: x) {
            y=0;
        }
    }


}

void assign_wavelength() {

//----------------------------------build up the overall connections---------------------------------------------
    //int overall_MEMS[IM_module_number][IM_module_number]= {0};	//build up the overall connection of MEMS

    for(int i=0; i<IM_module_number; i++) {
        for(int j=0; j<IM_module_number; j++) {
            for (int k=0; k<IM_port_number; k++) {
                //if(MEMS_connection[k][i][j]==1) {
                overall_MEMS[i][j]=1;
                //    break;
            }
        }
    }



    //for(const auto &x: overall_MEMS) {
    //    for(auto &y: x) {
    //        cout<< y <<"\t";
    //    }
    //    cout<<endl;
    //}

    //cout<<"Print the overall_MEMS matrix here"<<endl;

//--------------------------------------------------------------------------------------------------------------
    int each_ToR[ToR_number][2]; //The wavelength left for each of ToR's input & output(MN by 2)
    int IM_OM[IM_module_number][IM_module_number]; //The wavelength left for each fiber(M by M)
    int wavelength_avail[IM_port_number][IM_port_number];   //The temporary matrix for wavelength available in a fiber(N by N)
//int total_wavelength=0;

    for(auto &x: each_ToR) {
        for(auto &y: x) {
            y=wavelength;
        }
    }

    for(auto &x: IM_OM) {
        for(auto &y: x) {
            y=wavelength;
        }
    }

    /*Now we have all constraint with each_ToR for each ToR's input & output wavelength and IM_OM for each fiber*/
    /*We need to traverse every fiber to randomly assign a wavelength to a ToR-to-ToR traffic, meanwhile we got to check the constraint of the ToR input and ToR output wavelength number*/
    /*While there is no more wavelength can be assigned due to IM_OM, it will stop*/
    for(int IM=0; IM<IM_module_number; IM++) {
        for(int OM=0; OM<IM_module_number; OM++) {
//		here we have accessed to each of the fiber
//----------------------------------------------------------------------------------------------------------------------
            if(overall_MEMS[IM][OM]==0)
                break;
            //if this IM to OM connection has not been established, break

            //cout<<"Handling the "<<IM<<"th IM and "<<OM<<" th OM right here"<<endl;
            int total_wave=0;
            for(int i=0; i<IM_port_number; i++) {
                for(int j=0; j<IM_port_number; j++) {
                    wavelength_avail[i][j]=min(each_ToR[IM*IM_port_number+i][0],each_ToR[OM*IM_port_number+j][1]);
                    //wavelength_avail[i][j]=1;
                    total_wave+=wavelength_avail[i][j];
                }
            }//to check all ToR-to-ToR wavelength avilable

            //cout<<"here we calculated all available wavelength"<<endl;

            //use a random number generator to generate the source and destination ToR of the wavelength
            int wave_left=min(total_wave,wavelength);
            while(wave_left>0) {
                int location=random_number();
                int x=location/IM_port_number;
                int y=location%IM_port_number;
                if(wavelength_avail[x][y]>0) {
                    wave_left--;
                    total_wave--;
                    each_ToR[IM*IM_port_number+x][0]--;
                    each_ToR[OM*IM_port_number+y][1]--;
                    ToR_wavelength[IM*IM_port_number+x][OM*IM_port_number+y]++;

                    for(int k=0; k<IM_port_number; k++) {
                        if(wavelength_avail[x][k]>=wavelength_avail[x][y]&&y!=k) {
                            wavelength_avail[x][k]--;
                            total_wave--;
                        }
                        if(wavelength_avail[k][y]>=wavelength_avail[x][y]&&x!=k) {
                            wavelength_avail[k][y]--;
                            total_wave--;
                        }
                    }

                    wavelength_avail[x][y]--;

                }
                if(total_wave==0)
                    break;
            }
//----------------------------------------------------------------------------------------------------------------------
        }
    }


    return;
}

void construct_V_to_E_graph() {
    //Reform the ToR_wavelength's value in the matrix
    int V_index=0;
    for(auto &x: ToR_wavelength) {
        for(auto &y: x) {
            if (y) {
                y=V_index;
                V_index++;
            }
        }
    }

    //Set up a 2D-matrix vector< vector<> >
    vector<vector<int>> graph(V_index);

    //Here we want to traverse every fiber, and set up all E for one V each time
    for(int IM=0; IM<IM_module_number; IM++) {
        for(int OM=0; OM<IM_module_number; OM++) {
            vector<int> temp_vectors;

            if(overall_MEMS[IM][OM]==0)
                break;

            for(int i=0; i<IM_port_number; i++) {
                for(int j=0; j<IM_port_number; j++) {

                    int x=IM*IM_port_number+i;
                    int y=OM*IM_port_number+j;
                    int temp_number=ToR_wavelength[x][y];

                    if(temp_number) {  //if there is some a vertice in ToR x to ToR y, add it into the whole graph
                        //add all V in the temp_vector(same fiber) to a new edge to current node
                        for(auto &vertice: temp_vectors) {
                            graph[temp_number].push_back(vertice);
                            graph[vertice].push_back(temp_number);
                        }
                        temp_vectors.push_back(temp_number);

                        //add all V in the same column/row
                        for(int col=y+1; col<ToR_number; ++col) {
                            if(col/IM_port_number!=OM&&ToR_wavelength[x][col]) {
                                graph[temp_number].push_back(ToR_wavelength[x][col]);
                                graph[ToR_wavelength[x][col]].push_back(temp_number);
                            }
                        }

                        for(int row=x+1; row<ToR_number; ++row) {
                            if(row/IM_port_number!=IM&&ToR_wavelength[row][y]) {
                                graph[temp_number].push_back(ToR_wavelength[row][y]);
                                graph[ToR_wavelength[row][y]].push_back(temp_number);
                            }
                        }

                    }
                }
            }

        }
    }

    for(auto &x: graph) {
        sort(x.begin(),x.end());
    }

    string filename = "V_to_E_graph.txt";

    ofstream fileinput(filename.c_str(),ofstream::out | ofstream::app);


    for(const auto &x: graph) {
        for(auto &y: x) {
            fileinput<< y <<"\t";
        }
        fileinput<<endl;
    }
    fileinput.close();
    //Now V is as the index of every vector, and all int value inside it is its edge to another V
    //Graph is saved in V_to_E_graph.txt

}

int main(int argc, char **argv) {
    //construct_MEMS_connection();
    intialize();

    assign_wavelength();


    string filename = "ToR_wavelength.txt";

    ofstream fileinput(filename.c_str(),ofstream::out | ofstream::app);


    for(const auto &x: ToR_wavelength) {
        for(auto &y: x) {
            fileinput<< y <<"\t";
        }
        fileinput<<endl;
    }
    fileinput.close();



    construct_V_to_E_graph();

    //graph_coloring();

    return 0;
}

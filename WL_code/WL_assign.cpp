#include <iostream>
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>  
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

//global variables
int IM_size;
int MEMS_size;
int wavelen_num;
int ToR_num;
int conn_limit;
int fiber_num;
int edge_num;
int min_color_num = -1;

void print_usage(){
	cout << "========== usage ==========" << endl;
	cout << "./WL_assign -N IM_size -M MEMS_size -W wavelength_num -C conn_limit -m min_color_num" << endl;
	cout << "===========================" << endl;
}

bool if_MEMS_row_col(int **MEMS_conn, int row, int col){

	int count_row = 0;
	int count_col = 0;

	for (int i = 0 ; i < MEMS_size ; i++){
		if ( MEMS_conn[row][i] > 0 ){
			count_row += MEMS_conn[row][i];
		}
		if ( MEMS_conn[i][col] > 0 ){
			count_col += MEMS_conn[i][col];
		}
	}

	if (count_row >= IM_size || count_col >= IM_size){
		return true;
	}else{
		return false;
	}

}

void MEMS_connection_gen(int **MEMS_conn){

	int tot_conn = MEMS_size * IM_size;
	int reset_counter;
	bool reset_flag = false;

	do {

		reset_flag = false;

		for (int i = 0 ; i < tot_conn ; i++){

			int rand_row = rand() % MEMS_size;
			int rand_col = rand() % MEMS_size;

			reset_counter = 0;

			while ( MEMS_conn[rand_row][rand_col] >= conn_limit || if_MEMS_row_col(MEMS_conn, rand_row, rand_col) ) {

				if ( rand_col < ( MEMS_size - 1 ) ){
					rand_col++;
				}else{
					if ( rand_row < (MEMS_size - 1) ){
						rand_row++;
						rand_col = 0;
					}else{
						rand_row = 0;
						rand_col = 0;
					}
				}

				reset_counter++;
				if ( reset_counter > MEMS_size * MEMS_size ){
					reset_flag = true;
					break;
				}	

			}	

			if (reset_flag){
				for (int j = 0 ; j < MEMS_size ; j++){
					for (int k = 0 ; k < MEMS_size ; k++){
						MEMS_conn[j][k] = 0;
					}
				}
			}else{
				MEMS_conn[rand_row][rand_col]++;
			}

		}

	}while ( reset_flag );


	//printing out MEMS connection
	/*
	cout << endl;
	cout << "IM OM connection matrix" << endl;
	for (int i = 0 ; i < MEMS_size ; i++){
		for (int j = 0 ; j < MEMS_size ; j++){
			cout << MEMS_conn[i][j] << "\t";
		}
		cout << endl;
	}
	cout << endl;
	*/
}

void print_MEMS_conn(int **MEMS_conn){

	cout << endl;
	cout << "IM OM connection matrix" << endl;
	for (int i = 0 ; i < MEMS_size ; i++){
		for (int j = 0 ; j < MEMS_size ; j++){
			cout << MEMS_conn[i][j] << "\t";
		}
		cout << endl;
	}
	cout << endl;

}

void channel_assignment(int ***fiber_channel, int **fiber_conn){

	int **ToR_avail_w = new int *[ToR_num];
	for (int i = 0 ; i < ToR_num ; i++){
		ToR_avail_w[i] = new int [2];
		for (int j = 0 ; j < 2 ; j++){
			ToR_avail_w[i][j] = wavelen_num;
		}
	}

	int skip_counter = 0;

	for (int i = 0 ; i < fiber_num ; i++){
		for (int j = 0 ; j < wavelen_num ; j++){

			int src_ToR = rand() % IM_size;
			while( ToR_avail_w[ fiber_conn[i][0] * IM_size + src_ToR ][0] == 0 ){ // == 0 for normal, <= 1 for 2w - 1
				if ( src_ToR == (IM_size - 1) ){
					src_ToR = 0;
				}else{
					src_ToR++;
				}
			}
			int dst_ToR = rand() % IM_size;
			while( ToR_avail_w[ fiber_conn[i][1] * IM_size + dst_ToR ][1] == 0 ){ // == 0 for normal, <= 1 for 2w - 1
				if ( dst_ToR == (IM_size - 1) ){
					dst_ToR = 0;
				}else{
					dst_ToR++;
				}
			}	

			fiber_channel[i][j][0] = fiber_conn[i][0] * IM_size + src_ToR;
			fiber_channel[i][j][1] = fiber_conn[i][1] * IM_size + dst_ToR;
			ToR_avail_w[ fiber_conn[i][0] * IM_size + src_ToR ][0]--;
			ToR_avail_w[ fiber_conn[i][1] * IM_size + dst_ToR ][1]--;

		}
	}

	//print out 
	/*
	for (int i = 0 ; i < fiber_num ; i++){
		for (int j = 0 ; j < wavelen_num ; j++){
			cout << fiber_channel[i][j][0] << "\t" << fiber_channel[i][j][1] << endl;
		}
		cout << endl;
	}
	*/

}

void print_fiber_channel(int ***fiber_channel){

	for (int i = 0 ; i < fiber_num ; i++){
		for (int j = 0 ; j < wavelen_num ; j++){
			cout << fiber_channel[i][j][0] << "\t" << fiber_channel[i][j][1] << endl;
		}
		cout << endl;
	}

}

void print_fiber_channel_color(int ***fiber_channel){

	for (int i = 0 ; i < fiber_num ; i++){
		for (int j = 0 ; j < wavelen_num ; j++){
			cout << fiber_channel[i][j][0] << "\t" << fiber_channel[i][j][1] << "\t" << fiber_channel[i][j][2] << endl;
		}
		cout << endl;
	}

}

void computeSharedEdge(int **sharedEdge, int ***fiber_channel){

	vector < vector <int> > src_ToR;
	vector < vector <int> > dst_ToR ;
	for (int i = 0 ; i < ToR_num ; i++){
		vector <int> temp;
		src_ToR.push_back( temp );
		dst_ToR.push_back( temp );
	}

	for (int i = 0 ; i < fiber_num ; i++){
		for (int j = 0 ; j < wavelen_num ; j++){

			int edge_index = i * wavelen_num + j;

			src_ToR[ fiber_channel[i][j][0] ].push_back( edge_index );
			dst_ToR[ fiber_channel[i][j][1] ].push_back( edge_index );

			for (int k = 0 ; k < wavelen_num ; k++){

				int tmp_index = i * wavelen_num + k;

				sharedEdge[edge_index][tmp_index] = 1;
			}

		}
	}
	
	for (int i = 0 ; i < ToR_num ; i++){

		for (int j = 0 ; j < src_ToR[i].size() ; j++){
			for (int k = 0 ; k < src_ToR[i].size() ; k++){
				sharedEdge[ src_ToR[i][j] ][ src_ToR[i][k] ] = 1;
			}
		}

		for (int j = 0 ; j < dst_ToR[i].size() ; j++){
			for (int k = 0 ; k < dst_ToR[i].size() ; k++){
				sharedEdge[ dst_ToR[i][j] ][ dst_ToR[i][k] ] = 1;
			}
		}

	}

	for (int i = 0 ; i < edge_num ; i++){
		sharedEdge[i][i] = 0;
	}

	//print out
	/*
	for (int i = 0 ; i < edge_num ; i++){
		for (int j = 0 ; j < edge_num ; j++){
			cout << sharedEdge[i][j] << "\t";
		}
		cout << endl;
	}
	*/
	
}

int compute_ampl(int **sharedEdge){

	char cmd[256];
	
	string filename_data = "edge_color.dat"; 
	string filename_run = "edge_color.run"; 
	
	ofstream fout(filename_data, std::ofstream::trunc); 
	if(!fout) { 
	   	 cout << "can not write into the data file" << endl; 
   		 return -1; 
	} 

	fout << "data;" << endl;
	fout << "param C := " << wavelen_num << ";" << endl;
	fout << "param E := " << edge_num << ";" << endl;
	
	fout << endl;

	fout << "param s := ";
	for (int i = 0 ; i < edge_num ; i++){
		for (int j = 0 ; j < edge_num ; j++){
			fout << i << "\t" << j << "\t" << sharedEdge[i][j];
			if (i == (edge_num-1) && j == (edge_num-1)){
				fout << ";" << endl;
			}else{
				fout << endl;
			}
		}
	}
	
	fout << endl;
	
	fout << "end;" << endl;

	fout.close();

	sprintf(cmd, "ampl_macosx64/./ampl edge_color.run > tmp");
	system(cmd);

	ifstream tmp_file("tmp");
	
	if (!tmp_file){
		cout << "can not read/open the tmp_file file" << endl;
		return -1;
	}
	

	string aLine;
	getline(tmp_file, aLine);	
	istringstream iss(aLine);
	string sub;
	while (iss >> sub){
		if (sub == "objective"){
			iss >> sub;
			cout << sub << endl;
			break;
		}
	}

	tmp_file.close();

	return atoi(sub.c_str());

}

int least_common_avail_color(int *fiber, int *src, int *dst){

	int result = -1;

	for (int i = 0 ; i < wavelen_num ; i++){
		if ( fiber[i] == -1 && src[i] == -1 && dst[i] == -1 ){
			result = i;
			break;
		}
	}

	return result;

}

int contention_common(int *fiber, int *src, int *dst, int *color_list){

	int result = 2;

	for (int i = 0 ; i < wavelen_num ; i++){

		int counter = 0;

		if ( fiber[i] == -1 ) counter++;
		if ( src[i] == -1 )  counter++;
		if ( dst[i] == -1 )  counter++;

		if (counter >= 2){
			result = 1;

			if ( fiber[i] == -1 ){
				color_list[1] = i;
			}else{
				for (int j = 0 ; j < wavelen_num ; j++){
					if ( fiber[j] == -1 ){
						color_list[1] = j;
						break;
					}
				}
			}

			if ( src[i] == -1 ){
				color_list[0] = i;
			}else{
				for (int j = 0 ; j < wavelen_num ; j++){
					if ( src[j] == -1 ){
						color_list[0] = j;
						break;
					}
				}
			}

			if ( dst[i] == -1 ){
				color_list[2] = i;
			}else{
				for (int j = 0 ; j < wavelen_num ; j++){
					if ( dst[j] == -1 ){
						color_list[2] = j;
						break;
					}
				}
			}

			break;
		}

	}

	if (result == 2){
		for (int i = 0 ; i < wavelen_num ; i++){
			if ( fiber[i] == -1 ) color_list[1] = i;
			if ( src[i] == -1 ) color_list[0] = i;
			if ( dst[i] == -1 ) color_list[2] = i;
		}
	}

	return result;

}

//color_list: (src, fiber, dst)

bool swapping_color(int new_color, int old_color, int pos, bool **swap_flag, int ***fiber_channel, int fiber_index, int channel_index, int **fiber_colors, int **src_colors, int **dst_colors){

	bool result = true;

	switch (pos){
		case 0:
			cout << "at src ..." << endl;
			if ( fiber_channel[ fiber_index ][ channel_index ][2] == new_color ){
				if ( swap_flag[0][ fiber_channel[ fiber_index ][ channel_index ][0] ] ){
					return true;
				}else{				
					if ( src_colors[ fiber_channel[ fiber_index ][ channel_index ][0] ][new_color] != fiber_index * wavelen_num + channel_index ){
						int old_color_index = src_colors[ fiber_channel[ fiber_index ][ channel_index ][0] ][new_color];
						if (old_color_index == -1){
							src_colors[ fiber_channel[ fiber_index ][ channel_index ][0] ][new_color] = fiber_index * wavelen_num + channel_index;
							swap_flag[0][ fiber_channel[ fiber_index ][ channel_index ][0] ] = true;
							result = swapping_color(new_color, old_color, 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors);
						}else{
							src_colors[ fiber_channel[ fiber_index ][ channel_index ][0] ][new_color] = fiber_index * wavelen_num + channel_index;
							fiber_channel[ old_color_index / wavelen_num ][ old_color_index % wavelen_num ][2] = old_color;
							swap_flag[0][ fiber_channel[ fiber_index ][ channel_index ][0] ] = true;
							result = swapping_color(new_color, old_color, 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) &
									 swapping_color(old_color, new_color, 1, swap_flag, fiber_channel, old_color_index/wavelen_num, old_color_index%wavelen_num, fiber_colors, src_colors, dst_colors);
						}
					}else{
						swap_flag[0][ fiber_channel[ fiber_index ][ channel_index ][0] ] = true;
						return true;
					}
				}
			}else{
				if ( swap_flag[0][ fiber_channel[ fiber_index ][ channel_index ][0] ] ){
					return false;
				}else{
					int old_color = fiber_channel[ fiber_index ][ channel_index ][2];

					if (src_colors[ fiber_channel[fiber_index][channel_index][0] ][new_color] == -1){
						fiber_channel[ fiber_index ][ channel_index ][2] = new_color;
						src_colors[ fiber_channel[fiber_index][channel_index][0] ][new_color] = fiber_index * wavelen_num + channel_index;
						src_colors[ fiber_channel[fiber_index][channel_index][0] ][old_color] = -1;
						swap_flag[0][ fiber_channel[fiber_index][channel_index][0] ] = true;
						result = swapping_color(new_color, old_color, 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors); 
					}else{
						int new_color_index = src_colors[ fiber_channel[fiber_index][channel_index][0] ][new_color];
						fiber_channel[ fiber_index ][ channel_index ][2] = new_color;
						src_colors[ fiber_channel[fiber_index][channel_index][0] ][new_color] = fiber_index * wavelen_num + channel_index;
						fiber_channel[ new_color_index / wavelen_num ][ new_color_index % wavelen_num ][2] = old_color;
						src_colors[ fiber_channel[fiber_index][channel_index][0] ][old_color] = new_color_index;
						swap_flag[0][ fiber_channel[fiber_index][channel_index][0] ] = true;

						result = swapping_color(new_color, old_color, 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) & 
							 	 swapping_color(old_color, new_color, 1, swap_flag, fiber_channel, new_color_index/wavelen_num, new_color_index%wavelen_num, fiber_colors, src_colors, dst_colors);
					}

				}
			}
			break;
		case 1:
			cout << "at fiber ..." << endl;
			if ( fiber_channel[ fiber_index ][ channel_index ][2] == new_color ){
				if ( swap_flag[1][ fiber_index ] ){
					return true;
				}else{
					if ( fiber_colors[ fiber_index ][new_color] != fiber_index * wavelen_num + channel_index ){
							int old_color_index = fiber_colors[ fiber_index ][new_color];
							if (old_color_index == -1){
								fiber_colors[ fiber_index ][new_color] = fiber_index * wavelen_num + channel_index;
								swap_flag[1][ fiber_index ] = true;
								result = swapping_color(new_color, old_color, 0, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) &
										 swapping_color(new_color, old_color, 2, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors);
							}else{
								fiber_colors[ fiber_index ][new_color] = fiber_index * wavelen_num + channel_index;
								fiber_channel[ old_color_index / wavelen_num ][ old_color_index % wavelen_num ][2] = old_color;
								swap_flag[1][ fiber_index ] = true;
								result = swapping_color(new_color, old_color, 0, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) &
										 swapping_color(new_color, old_color, 2, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) &
										 swapping_color(old_color, new_color, 0, swap_flag, fiber_channel, old_color_index/wavelen_num, old_color_index%wavelen_num, fiber_colors, src_colors, dst_colors) &
										 swapping_color(old_color, new_color, 2, swap_flag, fiber_channel, old_color_index/wavelen_num, old_color_index%wavelen_num, fiber_colors, src_colors, dst_colors);
							}
					}else{
						swap_flag[1][ fiber_index ] = true;
						return true;
					}
				}
			}else{
				if ( swap_flag[1][ fiber_index ] ){
					return false;
				}else{
					//int old_color = fiber_channel[ fiber_index ][ channel_index ][2];

					if (fiber_colors[ fiber_index ][new_color] == -1){
						fiber_channel[ fiber_index ][ channel_index ][2] = new_color;
						fiber_colors[ fiber_index ][new_color] = fiber_index * wavelen_num + channel_index;
						fiber_colors[ fiber_index ][old_color] = -1;
						swap_flag[1][ fiber_index ] = true;
						result = swapping_color(new_color, old_color, 0, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) &
								 swapping_color(new_color, old_color, 2, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors);
					}else{
						int new_color_index = fiber_colors[ fiber_index ][new_color];
						fiber_channel[ fiber_index ][ channel_index ][2] = new_color;
						fiber_colors[ fiber_index ][new_color] = fiber_index * wavelen_num + channel_index;
						fiber_channel[ new_color_index / wavelen_num ][ new_color_index % wavelen_num ][2] = old_color;
						fiber_colors[ fiber_index ][old_color] = new_color_index;
						swap_flag[1][ fiber_index ] = true;

						result = swapping_color(new_color, old_color, 0, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) & 
								 swapping_color(new_color, old_color, 2, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) &
							 	 swapping_color(old_color, new_color, 0, swap_flag, fiber_channel, new_color_index/wavelen_num, new_color_index%wavelen_num, fiber_colors, src_colors, dst_colors) &
							 	 swapping_color(old_color, new_color, 2, swap_flag, fiber_channel, new_color_index/wavelen_num, new_color_index%wavelen_num, fiber_colors, src_colors, dst_colors);
					}

				}
			}
			break;
		case 2:
			cout << "at dst ..." << endl;
			if ( fiber_channel[ fiber_index ][ channel_index ][2] == new_color ){
				if ( swap_flag[2][ fiber_channel[ fiber_index ][ channel_index ][1] ] ){
					return true;
				}else{
					if ( dst_colors[ fiber_channel[ fiber_index ][ channel_index ][1] ][new_color] != fiber_index * wavelen_num + channel_index ){
							int old_color_index = dst_colors[ fiber_channel[ fiber_index ][ channel_index ][1] ][new_color];
							if (old_color_index == -1){
								dst_colors[ fiber_channel[ fiber_index ][ channel_index ][1] ][new_color] = fiber_index * wavelen_num + channel_index;
								swap_flag[2][ fiber_channel[ fiber_index ][ channel_index ][1] ] = true;
								result = swapping_color(new_color, old_color, 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors);
							}else{
								dst_colors[ fiber_channel[ fiber_index ][ channel_index ][1] ][new_color] = fiber_index * wavelen_num + channel_index;
								fiber_channel[ old_color_index / wavelen_num ][ old_color_index % wavelen_num ][2] = old_color;
								swap_flag[2][ fiber_channel[ fiber_index ][ channel_index ][1] ] = true;
								result = swapping_color(new_color, old_color, 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) &
										 swapping_color(old_color, new_color, 1, swap_flag, fiber_channel, old_color_index/wavelen_num, old_color_index%wavelen_num, fiber_colors, src_colors, dst_colors);
							}
					}else{
						swap_flag[2][ fiber_channel[ fiber_index ][ channel_index ][1] ] = true;
						return true;
					}
				}
			}else{
				if ( swap_flag[2][ fiber_channel[ fiber_index ][ channel_index ][1] ] ){
					return false;
				}else{
					int old_color = fiber_channel[ fiber_index ][ channel_index ][2];

					if (dst_colors[ fiber_channel[fiber_index][channel_index][1] ][new_color] == -1){
						fiber_channel[ fiber_index ][ channel_index ][2] = new_color;
						dst_colors[ fiber_channel[fiber_index][channel_index][1] ][new_color] = fiber_index * wavelen_num + channel_index;
						dst_colors[ fiber_channel[fiber_index][channel_index][1] ][old_color] = -1;
						swap_flag[2][ fiber_channel[fiber_index][channel_index][1] ] = true;
						result = swapping_color(new_color, old_color, 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors); 
					}else{
						int new_color_index = dst_colors[ fiber_channel[fiber_index][channel_index][1] ][new_color];
						fiber_channel[ fiber_index ][ channel_index ][2] = new_color;
						dst_colors[ fiber_channel[fiber_index][channel_index][1] ][new_color] = fiber_index * wavelen_num + channel_index;
						fiber_channel[ new_color_index / wavelen_num ][ new_color_index % wavelen_num ][2] = old_color;
						dst_colors[ fiber_channel[fiber_index][channel_index][1] ][old_color] = new_color_index;
						swap_flag[2][ fiber_channel[fiber_index][channel_index][1] ] = true;

						result = swapping_color(new_color, old_color, 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors) & 
							 	 swapping_color(old_color, new_color, 1, swap_flag, fiber_channel, new_color_index/wavelen_num, new_color_index%wavelen_num, fiber_colors, src_colors, dst_colors);
					}

				}
			}
			break;
		default:
			break;
	}

	return result;
}

void swapping_case2(int ***fiber_channel, int *color_list, int fiber_index, int channel_index, int **fiber_colors, int **src_colors, int **dst_colors){

	bool **swap_flag = new bool *[3];
	swap_flag[0] = new bool [ToR_num]; // for src
	swap_flag[1] = new bool [fiber_index]; // for fibers
	swap_flag[2] = new bool [ToR_num]; // for dst

	for (int i = 0 ; i < ToR_num ; i++){
		swap_flag[0][i] = false;
		swap_flag[2][i] = false;
	}
	for (int i = 0 ; i < fiber_num ; i++){
		swap_flag[1][i] = false;
	}

	int swap_result;
	//find out the single color as the starting point
	if ( color_list[0] == color_list[1] ){
		cout << "starting at dst" << endl;
		swap_flag[0][ fiber_channel[ fiber_index ][ channel_index ][0] ] = true;
		swap_flag[1][ fiber_index ] = true;
		fiber_channel[ fiber_index ][ channel_index ][2] = color_list[0];
		swap_result = swapping_color(color_list[0], color_list[2], 2, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors);
	}else if ( color_list[0] == color_list[2] ){
		cout << "starting at fiber" << endl;
		swap_flag[0][ fiber_channel[ fiber_index ][ channel_index ][0] ] = true;
		swap_flag[2][ fiber_channel[ fiber_index ][ channel_index ][1] ] = true;
		fiber_channel[ fiber_index ][ channel_index ][2] = color_list[0];
		swap_result = swapping_color(color_list[0], color_list[1], 1, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors);
	}else{
		cout << "starting at src" << endl;
		swap_flag[1][ fiber_index ] = true;
		swap_flag[2][ fiber_channel[ fiber_index ][ channel_index ][1] ] = true;
		fiber_channel[ fiber_index ][ channel_index ][2] = color_list[1];
		swap_result = swapping_color(color_list[1], color_list[0], 0, swap_flag, fiber_channel, fiber_index, channel_index, fiber_colors, src_colors, dst_colors);
	}

	if (swap_result){
		cout << "swap succeeded" << endl;
	}else{
		cout << "swap failed" << endl;
	}

}

void heuristic_coloring(int ***fiber_channel){

	int **src_colors = new int *[ToR_num];
	int **dst_colors = new int *[ToR_num];
	int **fiber_colors = new int *[fiber_num];

	for (int i = 0 ; i < ToR_num ; i++){
		src_colors[i] = new int [wavelen_num];
		dst_colors[i] = new int [wavelen_num];
		for (int j = 0 ; j < wavelen_num ; j++){
			src_colors[i][j] = -1;
			dst_colors[i][j] = -1;
		}
	}
	for (int i = 0 ; i < fiber_num ; i++){
		fiber_colors[i] = new int [wavelen_num];
		for (int j = 0 ; j < wavelen_num ; j++){
			fiber_colors[i][j] = -1;
		}
	}

	//start to assign color to each channel, (fiber, lambda)
	for (int i = 0 ; i < fiber_num ; i++){

		for (int j = 0 ; j < wavelen_num ; j++){

			int channel_index = i * wavelen_num + j;

			int avail_color = least_common_avail_color(fiber_colors[i], src_colors[ fiber_channel[i][j][0] ], dst_colors[ fiber_channel[i][j][1] ]);

			if (avail_color == -1){ //oops, there is contention

				//we need to tell if it's case 1 or case 2 contention
				//case1: two two of them can use the same color, ex: (1, 1, 2)
				//case2: all have to use different colors, ex: (1, 2, 3)
				int *color_list = new int [3];

				if ( contention_common(fiber_colors[i], src_colors[ fiber_channel[i][j][0] ], dst_colors[ fiber_channel[i][j][1] ], color_list) == 1 ){
					//case1
					cout << "now processing case 1" << endl;

					swapping_case2(fiber_channel, color_list, i, j, fiber_colors, src_colors, dst_colors);

				}else{
					//case2
					cout << "now processing case 2" << endl;

				}

			}else{ //oh yeah, no contention, let's just use this color

				fiber_colors[i][avail_color] = channel_index;
				src_colors[ fiber_channel[i][j][0] ][avail_color] = channel_index;
				dst_colors[ fiber_channel[i][j][1] ][avail_color] = channel_index;
				fiber_channel[ i ][ j ][ 2 ] = avail_color;

			}

		}

	}


	//filling in value for fiber_channel[i][j][2]
	//for (int i = 0 ; i < fiber_num ; i++){
	//	for (int j = 0 ; j < wavelen_num ; j++){
	//		if ( fiber_colors[i][j] != -1 ){
	//			fiber_channel[ fiber_colors[i][j] / fiber_num ][ fiber_colors[i][j] % fiber_num ][2] = j;
	//		}
	//	}
	//}

	for (int i = 0 ; i < ToR_num ; i++){
		delete src_colors[i];
		delete dst_colors[i];
	}
	delete src_colors;
	delete dst_colors;
	for (int i = 0 ; i < fiber_num ; i++){
		delete fiber_colors[i];
	}
	delete fiber_colors;

}

int main(int argc, char **argv){

	srand (time(NULL));

	//default settings for input parameters
	IM_size = 4;
	MEMS_size = 16;
	wavelen_num = 10;
	conn_limit = 1;

	//set input parameters
	for(int i = 1; i < argc; i++){
        if(argv[i][0] != '-'){
            print_usage();  
            return 0;
        }
        switch(argv[i][1]){
            case 'N': 
				IM_size = atoi(argv[++i]); 
				break;
			case 'M': 
				MEMS_size = atoi(argv[++i]); 
				break;
			case 'W': 
				wavelen_num = atoi(argv[++i]); 
				break;
			case 'C': 
				conn_limit = atoi(argv[++i]); 
				break;
			case 'm': 
				min_color_num = atoi(argv[++i]); 
				break;
            default:
            	print_usage(); 
            	return 0;
                break;
        }
    }
    ToR_num = IM_size * MEMS_size;
    fiber_num = IM_size * MEMS_size;
    //=================================

    //deciding MEMS connections
	int **MEMS_conn = new int *[MEMS_size];
	for (int i = 0 ; i < MEMS_size ; i++){
		MEMS_conn[i] = new int [MEMS_size];
		for (int j = 0 ; j < MEMS_size ; j++){
			MEMS_conn[i][j] = 0;
		}
	}

	MEMS_connection_gen(MEMS_conn);

	//assigning the channels (src ToR to dst ToR) for each fiber
	int **fiber_conn = new int *[fiber_num];
	for (int i = 0 ; i < fiber_num ; i++){
		fiber_conn[i] = new int [2];
	}
	//set fiber_conn, [0]: src module, [1]: dst module
	int counter = 0;
	for (int i = 0 ; i < MEMS_size ; i++){
		for (int j = 0 ; j < MEMS_size ; j++){
			for (int k = 0 ; k < MEMS_conn[i][j] ; k++){
				fiber_conn[ counter ][0] = i;
				fiber_conn[ counter ][1] = j;
				counter++;
			}
		}
	}
	//cout << "counter: " << counter << endl;

	int ***fiber_channel = new int **[fiber_num];
	for (int i = 0 ; i < fiber_num ; i++){
		fiber_channel[i] = new int *[wavelen_num];
		for (int j = 0 ; j < wavelen_num ; j++){
			fiber_channel[i][j] = new int [3];
			fiber_channel[i][j][2] = -1;
		}
	}

	channel_assignment(fiber_channel, fiber_conn);
	print_fiber_channel(fiber_channel);
	
	//trying to color all the channels 
	edge_num = fiber_num * wavelen_num;
	int **sharedEdge = new int *[edge_num];
	for (int i = 0 ; i < edge_num ; i++){
		sharedEdge[i] = new int [edge_num];
		for (int j = 0 ; j < edge_num ; j++){
			sharedEdge[i][j] = 0;
		}
	}

	computeSharedEdge(sharedEdge, fiber_channel);

	//write and feed ampl
	if ( compute_ampl(sharedEdge) <= min_color_num ){
	//	print_MEMS_conn(MEMS_conn);
	//	print_fiber_channel(fiber_channel);
	}

	heuristic_coloring(fiber_channel);

	print_fiber_channel_color(fiber_channel);


	//free memory
	for (int i = 0 ; i < MEMS_size ; i++){
		delete MEMS_conn[i];
	}
	for (int i = 0 ; i < fiber_num ; i++){
		delete fiber_conn[i];
		for (int j = 0 ; j < wavelen_num ; j++){
			delete fiber_channel[i][j];
		}
		delete fiber_channel[i];
	}

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>

/* The argument now should be a double (not a pointer to a double) */
#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#endif

#define TOTAL_CITIES 1000
#define MAX_IMPROVEMENTS 10

/* Function handles 
    *tour --> pointer to a tour (list of cities)
    start --> starting index
    end --> end index
    **matrix --> distance matrix
    *traversal_order --> current starting city traversal order
    *tour_lens --> Holds each starting city tour length
*/

long long check_tour(int *tour, long long **matrix);
void reverse(int *tour, int start, int end);
void two_five_opt(int *tour, int city, long long *tour_lens, long long **matrix);
long long compute_2opt_gain(int *tour, int i, int j, long long **matrix);
void apply_2opt_move(int *tour, int i, int j);
void apply_1node_shift(int *tour, int from, int to);
long long compute_1node_shift_gain(int *tour, int from, int to, long long **matrix);
double avg_outbound(int city, long long** matrix);
long long nearest_neighbor(long long **matrix, int start, int *traversal_order, int **tours, long long *tour_lens);

int main(){

    /* Starting variables */
    bool unvisited[TOTAL_CITIES] = {0};
    bool starting_used[TOTAL_CITIES] = {0};
    double avg_outbounds[TOTAL_CITIES];
    int best_order[TOTAL_CITIES];
    int traversal_order[TOTAL_CITIES];
    int tsp_alloted_time = 60;
    double lowest_outbound = 1e9;
    int starting_city = 0;
    double start, finish;
    long long **matrix = (long long**)malloc(sizeof(long long *) * TOTAL_CITIES);
    int **tours = (int**)malloc(sizeof(int *) * TOTAL_CITIES);
    long long *tour_lens = (long long *)malloc(sizeof(long long) * TOTAL_CITIES);
    FILE *f = fopen("DistanceMatrix1000_v2.csv", "r");
    // avg_outbounds[i] = avg_outbound(i, matrix);
    //     if(avg_outbounds[i] < lowest_outbound){
    //         lowest_outbound = avg_outbounds[i];
    //         starting_city = i;
    //     }
    /*
        1. Allocate submatrix i
        2. initialize unvisited and traversal order to 0
        3. Parse csv file line i
        4. Calculate avg outbound distance for city i
    */
    GET_TIME(start);
    for(int i = 0; i < TOTAL_CITIES; i++){
        matrix[i] = (long long*)malloc(sizeof(long long) * TOTAL_CITIES);
        starting_used[i] = false;
        for(int j = 0; j < TOTAL_CITIES; j++){
            if(j < TOTAL_CITIES-1){
                if(fscanf(f, "%lld,", &matrix[i][j]) != 1){
                    fprintf(stderr, "Error reading matrix data");
                    fclose(f);
                    exit(1);
                }
            } else {
                if(fscanf(f, "%lld", &matrix[i][j]) != 1){
                    fprintf(stderr, "Error reading matrix data");
                    fclose(f);
                    exit(1);
                }
            }
        }
    }
    
    /* Calculate first tour starting with the city with the lowest average outbound distance */
    starting_used[starting_city] = true;
    long long best_tour = nearest_neighbor(matrix, starting_city, traversal_order, tours, tour_lens);
    time_t endwait = time(NULL) + tsp_alloted_time;
    while(1){
        starting_city = -1;
        lowest_outbound = 1e9;
        for(int i = 0; i < TOTAL_CITIES; i++){
            if(avg_outbounds[i] < lowest_outbound && !starting_used[i]){
                starting_city = i;
                lowest_outbound = avg_outbounds[i];
            }
        }
        if (starting_city == -1) break;
        long long new_tour = nearest_neighbor(matrix, starting_city, traversal_order, tours, tour_lens);
        if(new_tour < best_tour) {
            best_tour = new_tour;
            memcpy(best_order, traversal_order, sizeof(int)*TOTAL_CITIES);
        }
        starting_used[starting_city] = true;
    }
    GET_TIME(start);
    int z = 0;

    GET_TIME(finish);
    /* Printing to stdout and cleanup */
    printf("Elapsed time: %.15e\n", finish-start);
    printf("Best tour found in %d alloted time Path: %lld\n", tsp_alloted_time, best_tour);

    fclose(f);
    for(int i = 0; i < TOTAL_CITIES; i++){
        free(matrix[i]);
        free(tours[i]);
    }
    free(tours);
    free(matrix);
    return 0;
}

long long check_tour(int *tour, long long **matrix){
    long long len = 0;
    for(int i = 0; i < TOTAL_CITIES * 2 - 1; i++){
        len+=matrix[tour[i]][tour[i+1]];
    }
    len+=matrix[tour[TOTAL_CITIES-1]][tour[0]];
    return len;
}

void two_five_opt(int *tour, int city, long long *tour_lens, long long **matrix){
    for(int i = 0; i < TOTAL_CITIES - 3; i++){
        int A = tour[i], B = tour[i+1];
        int z = matrix[A][B];
        
    }
}

// void two_five_opt(int *tour, int city, long long *tour_lens, long long **matrix){
//     bool improved = true;
//     int num_improvements = 0;
//     long long tour_len = tour_lens[city];
//     while(improved && num_improvements < MAX_IMPROVEMENTS){
//         improved = false;
//         for(int i = 0; i < TOTAL_CITIES-1 && num_improvements < MAX_IMPROVEMENTS; i++){
//             for(int j = i + 2; j < TOTAL_CITIES-1 && num_improvements < MAX_IMPROVEMENTS; j++){
//                 if(i == 0 && (j == (TOTAL_CITIES-1))){
//                     continue;
//                 }
//                 long long delta = compute_2opt_gain(tour, i, j, matrix);
//                 if (delta < 0){
//                     apply_2opt_move(tour, i, j);
//                     improved = true;
//                     tour_len+=delta;
//                     num_improvements++;
//                 }
//             }
//         }
//         // for(int i = 1; i < TOTAL_CITIES - 2 && num_improvements < MAX_IMPROVEMENTS; i++){
//         //     for(int j = 0; j < TOTAL_CITIES && num_improvements < MAX_IMPROVEMENTS; j++){
//         //         if(j == i || j == (i - 1) || j == (i+1)){
//         //             continue;
//         //         }
//         //         long long delta = compute_1node_shift_gain(tour, i, j, matrix);
//         //         if(delta < 0){
//         //             printf("1-opt gain %lld at (%d,%d)\n", delta, i, j);
//         //             apply_1node_shift(tour, i, j);
//         //             improved = true;
//         //             tour_len+=delta;
//         //             num_improvements++;
//         //             if(num_improvements == MAX_IMPROVEMENTS) break;
//         //         }
//         //     }
//         // }
//     }
//     tour_lens[city] = tour_len;
// }

long long compute_2opt_gain(int *tour, int i, int j, long long **matrix){
    long long A = tour[i];
    long long B = tour[(i+1) % TOTAL_CITIES];
    long long C = tour[j];
    long long D = tour[(j+1) % TOTAL_CITIES];

    long long old_cost = matrix[A][B] + matrix[C][D];
    long long new_cost = matrix[A][C] + matrix[B][D]; 
    for(int k = i+1; k < j; k++){
        old_cost += matrix[tour[k]][tour[k+1]];
        new_cost += matrix[tour[k+1]][tour[k]];
    }
    return new_cost - old_cost;
}

void apply_2opt_move(int *tour, int i, int j){
    reverse(tour, i+1, j);
}

void reverse(int *tour, int start, int end){
    while(start < end){
        int tmp = tour[start];
        tour[start++] = tour[end];
        tour[end--] = tmp;
    }
}

void apply_1node_shift(int *tour, int from, int to){
    int city = tour[from];
    if(from < to){
        for(int i = from; i < to; i++){
            tour[i] = tour[i+1];
        }
        tour[to] = city;
    } else {
        for(int i = from; i > to; i--){
            tour[i] = tour[i-1];
        }
        tour[to] = city;
    }
}

long long compute_1node_shift_gain(int *tour, int from, int to, long long **matrix){
    if(from == to || to == from - 1 || to == from + 1) return 0;

    long long A = tour[(from + TOTAL_CITIES - 1) % TOTAL_CITIES];
    long long B = tour[from];
    long long C = tour[(from + 1) % TOTAL_CITIES];
    long long D, E;
    if (to < from){
        D = tour[(to + TOTAL_CITIES - 1) % TOTAL_CITIES];
        E = tour[to];
    }
    else {
        D = tour[to];
        E = tour[(to + 1) % TOTAL_CITIES];
    }
    long long old_cost = matrix[A][B] + matrix[B][C] + matrix[D][E];
    long long new_cost = matrix[A][C] + matrix[D][B] + matrix[B][E];
    return new_cost - old_cost;
}

double avg_outbound(int city, long long **matrix){
    long long sum = 0;
    for(int i = 0; i < TOTAL_CITIES; i++){
        if(i != city){
            sum+=matrix[city][i];
        }
    }
    return sum / (double)(TOTAL_CITIES - 1);
}

long long nearest_neighbor(
        long long **matrix, 
        int start, 
        int *traversal_order, 
        int **tours,
        long long *tour_lens
    ){

    bool unvisited[TOTAL_CITIES] = {0};
    int curr_city = start;
    unvisited[start] = true;
    traversal_order[0] = start;
    long long sum = 0;

    for(int k = 1; k < TOTAL_CITIES; k++){
        long long shortest_edge = 1e9;
        int closest_city = -1;
        for(int i = 0; i < TOTAL_CITIES; i++){
            if(matrix[curr_city][i] < shortest_edge && !unvisited[i]){
                closest_city = i;
                shortest_edge = matrix[curr_city][i];
            }
        }
        traversal_order[k] = closest_city;
        unvisited[closest_city] = true;
        sum += shortest_edge;
        curr_city = closest_city;
    }
    traversal_order[TOTAL_CITIES-1] = start;
    sum+=matrix[curr_city][start];
    tour_lens[start] = sum;

    tours[start] = malloc(sizeof(int)*TOTAL_CITIES);
    memcpy(tours[start], traversal_order, sizeof(int)*TOTAL_CITIES);
    return sum;
}
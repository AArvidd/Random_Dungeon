#include "raylib.h"
#include "stdio.h"
#include "time.h"

#define width 100
#define height 100

int map1[width][height];
int map2[width][height];

int spatern_x[] = {-1, -1,  0,  1,  1,  1,  0, -1};
int spatern_y[] = { 0, -1, -1, -1,  0,  1,  1,  1};

void map_gen(){
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            int temp = GetRandomValue(1, 100);
            if(temp >= 70){
                map1[x][y] = 1;
            }else{
                map1[x][y] = 0;
            }
        }
    }

    for(int round = 0; round < 5; round++){
        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){

                int neighbors = 0;
                for(int i = 0; i < 8; i++){
                    if(x + spatern_x[i] < 0 || x + spatern_x[i] >= width || y + spatern_y[i] < 0 || y + spatern_y[i] >= width){
                        neighbors++;
                    }else if(map1[x + spatern_x[i]][y + spatern_y[i]]){
                        neighbors++;
                    }
                }
                if(neighbors >= 4){
                    map2[x][y] = 1;
                }else if(neighbors == 3){
                    map2[x][y] = map1[x][y];
                }else{
                    map2[x][y] = 0;
                }

            }
        }

        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){
                map1[x][y] = map2 [x][y];
            }
        }

    }
}

int main(){

    SetRandomSeed(time(NULL));

    map_gen();

    printf("       ");
    for(int x = 0; x < width; x++){
        printf("%2d ", x);
    }
    printf("\n");

    for(int y = 0; y < height; y++){
        printf("y = %2d:", y);
        for(int x = 0; x < width; x++){
            if(map1[x][y] == 1){
                printf(" # ");
            }else{
                printf(" . ");
            }
        }
        printf("\n");
    }

    return 0;
}
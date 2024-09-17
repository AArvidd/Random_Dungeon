#include "raylib.h"
#include "stdio.h"
#include "time.h"
#include "locale.h"
#include "wchar.h"

#define width 100
#define height 100

#define queue_lenth 200

#define search_cross 4
#define search_full 8

typedef enum{
    floor,
    rock,
    hardrock,
    wall,
    door,
}tile_type;

typedef struct{
    int curent;
    int next_empty;
    int queue_x[queue_lenth];
    int queue_y[queue_lenth];
}queue_t;

int map1[width][height];
int map2[width][height];

int spatern_x[] = {-1,  1,  0,  0, -1,  1, -1,  1};
int spatern_y[] = { 0,  0, -1,  1, -1, -1,  1,  1};

int wallchar[] = {0x256c, 0x2550, 0x2550, 0x2550, 0x2551, 0x255d, 0x255a, 0x2569, 0x2551, 0x2557, 0x2554, 0x2566, 0x2551, 0x2563, 0x2560, 0x256c};



queue_t queue = {
    .curent = 0,
    .next_empty = 0
};


int flood_search(int x, int y, int marker){
    int area = 1;
    queue.queue_x[0] = x;
    queue.queue_y[0] = y;
    queue.next_empty++;
    map2[x][y] = 1;
    while(queue.curent != queue.next_empty){
        for(int i = 0; i < search_cross; i++){
            if((
                map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == floor || 
                map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == door) && 
                map2[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] != marker
            ){
                map2[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] = marker;
                area++;
                queue.queue_x[queue.next_empty] = queue.queue_x[queue.curent] + spatern_x[i];
                queue.queue_y[queue.next_empty] = queue.queue_y[queue.curent] + spatern_y[i];
                queue.next_empty++;
                if(queue.next_empty == queue_lenth){
                    queue.next_empty = 0;
                }

            }
        }
        queue.queue_x[queue.curent] = 0;
        queue.queue_y[queue.curent] = 0;
        queue.curent++;
        if(queue.curent == queue_lenth){
            queue.curent = 0;
        }
    }
    queue.curent = 0;
    queue.next_empty = 0;
    return area;
}

void flood_remove(int x, int y){
    queue.queue_x[0] = x;
    queue.queue_y[0] = y;
    queue.next_empty++;
    map1[x][y] = 1;
    while(queue.curent != queue.next_empty){
        for(int i = 0; i < search_cross; i++){
            if(map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == floor){
                map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] = 1;
                queue.queue_x[queue.next_empty] = queue.queue_x[queue.curent] + spatern_x[i];
                queue.queue_y[queue.next_empty] = queue.queue_y[queue.curent] + spatern_y[i];
                queue.next_empty++;
                if(queue.next_empty == queue_lenth){
                    queue.next_empty = 0;
                }

            }
        }
        queue.queue_x[queue.curent] = 0;
        queue.queue_y[queue.curent] = 0;
        queue.curent++;
        if(queue.curent == queue_lenth){
            queue.curent = 0;
        }
    }
    queue.curent = 0;
    queue.next_empty = 0;
}

void add_hardrock(){
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){

            if(map1[x][y] == 0){
                continue;
            }

            int deap = 1;
            for(int x_off = -3; x_off < 4; x_off++){
                for(int y_off = -3; y_off < 4; y_off++){

                    if(x + x_off < 0 || x + x_off >= width || y + y_off < 0 || y + y_off >= height){
                        continue;
                    }else if(map1[x + x_off][y + y_off] == floor){
                        deap = 0;
                        goto next;
                    }

                }
            }

            next:;

            if(deap){
                map1[x][y] = hardrock;
            }

        }
    }
}

void cave_gen(){
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            int temp = GetRandomValue(1, 100);
            if(temp >= 70){
                map1[x][y] = rock;
            }else{
                map1[x][y] = floor;
            }
        }
    }

    for(int round = 0; round < 5; round++){
        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){

                int neighbors = 0;
                for(int i = 0; i < search_full; i++){
                    if(x + spatern_x[i] < 0 || x + spatern_x[i] >= width || y + spatern_y[i] < 0 || y + spatern_y[i] >= height){
                        neighbors++;
                    }else if(map1[x + spatern_x[i]][y + spatern_y[i]]){
                        neighbors++;
                    }
                }
                if(neighbors >= 4){
                    map2[x][y] = rock;
                }else if(neighbors == 3){
                    map2[x][y] = map1[x][y];
                }else{
                    map2[x][y] = floor;
                }

            }
        }

        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){
                map1[x][y] = map2 [x][y];
            }
        }
    }

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            map2[x][y] = 0;
        }
    }

    int remove_x[20];
    int remove_y[20];
    int remove = 0;

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            if(map1[x][y] == 0 && map2[x][y] != 1){
                int area = flood_search(x, y, 1);
                if(area < 11){
                    remove_x[remove] = x;
                    remove_y[remove] = y;
                    remove++;
                }
            }
        }
    }

    for(int i = 0; i < remove; i++){
        flood_remove(remove_x[i], remove_y[i]);
    }

    add_hardrock();

}

void coredor_gen(int from_x, int from_y, int to_x, int to_y){

    int moledir_x = to_x - from_x;
    int moledir_y = to_y - from_y;

    if(moledir_x < 0){
        moledir_x = -1;
    }else if(moledir_x > 0){
        moledir_x = 1;
    }

    if(moledir_y < 0){
        moledir_y = -1;
    }else if(moledir_y > 0){
        moledir_y = 1;
    }

    int olde_x = from_x;
    int olde_y = from_y;
    while(from_x != to_x || from_y != to_y){
        int oldest_x = olde_x;
        int oldest_y = olde_y;
        olde_x = from_x;
        olde_y = from_y;
        if(from_x != to_x){
            from_x += moledir_x;
        } else{
            from_y += moledir_y;
        }

        if(map1[from_x][from_y] == wall || map1[from_x][from_y] == door){
            map1[from_x][from_y] = door;
        }else{
            map1[from_x][from_y] = floor;
        }

        for(int i = 0; i < search_full; i++){
            if(map1[oldest_x + spatern_x[i]][oldest_y + spatern_y[i]] == rock){
                map1[oldest_x + spatern_x[i]][oldest_y + spatern_y[i]] = wall;
            }
        }
    }
}

void dungeon_gen(){
    for(int x = 0; x <width + 1; x++){
        for(int y = 0; y <height + 1 ; y++){
            map1[x][y] = 1;
        }
    }


    int rooms_x[10];
    int rooms_y[10];
    int rooms = 0;

    int attempts = 0;
    while(rooms < 10){
        int room_width = GetRandomValue(5, 15);
        int room_height = GetRandomValue(5, 15);
        int room_x = GetRandomValue(1, width - 1);
        int room_y = GetRandomValue(1, height - 1);

        int clear = 1;

        for(int x = -1; x < room_width + 1; x++){
            for(int y = -1; y < room_height + 1; y++){
                if(map1[room_x + x][room_y + y] == 0 || room_x + x < 0 || room_x + x >= width || room_y + y < 0 || room_y + y >= height){
                    clear = 0;
                    attempts++;
                    goto exit;
                }
            }
        }
        exit:;

        if(attempts == 10){
            break;
        }
        if(!clear){
            continue;
        }

        rooms_x[rooms] = room_x + (int)(room_width / 2);
        rooms_y[rooms] = room_y + (int)(room_height / 2);

        for(int x = -1; x <= room_width; x++){
            for(int y = -1; y <= room_height; y++){
                map1[room_x + x][room_y + y] = floor;
                if(x == -1 || x == room_width || y == -1 || y == room_height){
                    map1[room_x + x][room_y + y] = wall;
                }
            }
        }
        rooms++;
        attempts = 0;
    }

    for(int i = 0; i < rooms - 1; i++){
        int room1 = GetRandomValue(0, rooms - 1);
        int room2;
        while((room2 = GetRandomValue(0, rooms - 1)) == room1);

        coredor_gen(rooms_x[room1], rooms_y[room1], rooms_x[room2], rooms_y[room2]);
    }

    int areas = 0;
    while(1){
        areas = 0;
        for(int i = 0; i < rooms; i++){
            if(map2[rooms_x[i]][rooms_y[i]] == 0){
                areas++;
                flood_search(rooms_x[i], rooms_y[i], areas);
            }
        }

        if(areas == 1){
            break;
        }

        int room1 = GetRandomValue(0, rooms - 1);
        int room2;
        while((room2 = GetRandomValue(0, rooms - 1)) == room1);

        if(map2[rooms_x[room1]][rooms_y[room1]] != map2[rooms_x[room2]][rooms_y[room2]]){
            coredor_gen(rooms_x[room1], rooms_y[room1], rooms_x[room2], rooms_y[room2]);
        }

        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){
                map2[x][y] = 0;
            }
        }
    }

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            if(map1[x][y] == door){
                if( (map1[x + spatern_x[0]][y + spatern_y[0]] == wall && map1[x + spatern_x[1]][y + spatern_y[1]] == wall) || 
                    (map1[x + spatern_x[2]][y + spatern_y[2]] == wall && map1[x + spatern_x[3]][y + spatern_y[3]] == wall)
                ){
                    map1[x][y] = door;
                }else{
                    map1[x][y] = floor;
                }
            }
        }
    }
    

}

void map_gen(){
    dungeon_gen();
}

void draw_wall(int x, int y){
    int type = 0;

    for(int i = 0; i < search_cross; i++){
        if(map1[x + spatern_x[i]][y + spatern_y[i]] == wall || map1[x + spatern_x[i]][y + spatern_y[i]] == door){
            type |= 1 << i;
        }
    }

    if(type & 1){
        wprintf(L"%lc", wallchar[1]);
    }else{
        printf(" ");
    }

    wprintf(L"%lc", wallchar[type]);

    if(type & 2){
        wprintf(L"%lc", wallchar[1]);
    }else{
        printf(" ");
    }
    
}


int main(){

    setlocale(LC_ALL, "en_US.UTF-8");

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
            switch(map1[x][y]){
                case floor:
                    printf("   ");
                    break;
                case rock:
                    printf(" # ");
                    break;
                case hardrock:
                    printf(" & ");
                    break;
                case wall:
                    draw_wall(x, y);
                    break;
                case door:
                    printf(" D ");
                    break;
            }

        }
        printf("\n");
    }

    return 0;
}
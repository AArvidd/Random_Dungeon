#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "locale.h"
#include "wchar.h"
#include "termios.h"
#include "unistd.h"

#define width 100
#define height 100

#define world_width 50
#define world_height 50

#define queue_lenth 200

#define search_cross 4
#define search_full 8

typedef enum{
    tile_floor,
    tile_rock,
    tile_hardrock,
    tile_wall,
    tile_door,
    tile_exit,
}tile_type;

typedef enum{
    upp,
    down,
    left,
    right,
}direktions;

typedef struct{
    int curent;
    int next_empty;
    int queue_x[queue_lenth];
    int queue_y[queue_lenth];
}queue_t;

typedef struct{
    int exits_x[4];
    int exits_y[4];
    int viseted;
    int sead;
    int conection;
}world_tile_t;

typedef struct{
    int x;
    int y;
}player_t;

int map1[width][height];
int map2[width][height];

int world_pos_x = 5;
int world_pos_y = 5;

world_tile_t world_map[50][50];

player_t player;

int spatern_x[] = {-1,  1,  0,  0, -1,  1, -1,  1};
int spatern_y[] = { 0,  0, -1,  1, -1, -1,  1,  1};

int wallchar[] = {0x256c, 0x2550, 0x2550, 0x2550, 0x2551, 0x255d, 0x255a, 0x2569, 0x2551, 0x2557, 0x2554, 0x2566, 0x2551, 0x2563, 0x2560, 0x256c};



queue_t queue = {
    .curent = 0,
    .next_empty = 0
};

//world map generation
void hunt_and_kill(int current_x, int current_y){
    while(1){
        world_map[current_x][current_y].viseted = 1;
        int dir = rand() % 4;

        int tests = 0;
        while(1){
            if( world_map[current_x + spatern_x[dir]][current_y + spatern_y[dir]].viseted ||
                current_x + spatern_x[dir] < 0 || current_x + spatern_x[dir] > world_width - 1 ||
                current_y + spatern_y[dir] < 0 || current_y + spatern_y[dir] > world_height - 1
            ){
                tests++;
                if(tests == 4){
                    return;
                }
                dir++;
                if(dir == 4){
                    dir = 0;
                }
                continue;
            }
            break;
        }

        world_map[current_x][current_y].conection |= 1 << dir;

        current_x += spatern_x[dir];
        current_y += spatern_y[dir];

        switch(dir){
            case 0:
                world_map[current_x][current_y].conection |= 1 << 1;
                break;
            case 1:
                world_map[current_x][current_y].conection |= 1 << 0;
                break;
            case 2:
                world_map[current_x][current_y].conection |= 1 << 3;
                break;
            case 3:
                world_map[current_x][current_y].conection |= 1 << 2;
                break;
        }
    }
}

void world_map_gen(){
    srand(time(NULL));
    for(int y = 0; y < world_height; y++){
        for(int x = 0; x < world_width; x++){
            for(int i = 0; i < 4; i++){
                if(!world_map[x + spatern_x[i]][y + spatern_y[i]].viseted && !(
                    x + spatern_x[i] < 0 || x + spatern_x[i] > world_width - 1 ||
                    y + spatern_y[i] < 0 || y + spatern_y[i] > world_height - 1
                )){
                    hunt_and_kill(x, y);
                    x = 0;
                    y = 0;
                    break;
                }
            }
        }
    }
}

//map generation
int flood_search(int x, int y, int marker){
    int area = 1;
    queue.queue_x[0] = x;
    queue.queue_y[0] = y;
    queue.next_empty++;
    map2[x][y] = 1;
    while(queue.curent != queue.next_empty){
        for(int i = 0; i < search_cross; i++){
            if((
                map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == tile_floor || 
                map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == tile_door) && 
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
            if(map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == tile_floor){
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
                    }else if(map1[x + x_off][y + y_off] == tile_floor){
                        deap = 0;
                        goto next;
                    }

                }
            }

            next:;

            if(deap){
                map1[x][y] = tile_hardrock;
            }

        }
    }
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

        if(map1[from_x][from_y] == tile_wall || map1[from_x][from_y] == tile_door){
            map1[from_x][from_y] = tile_door;
        }else{
            map1[from_x][from_y] = tile_floor;
        }

        for(int i = 0; i < search_full; i++){
            if(map1[oldest_x + spatern_x[i]][oldest_y + spatern_y[i]] == tile_rock){
                map1[oldest_x + spatern_x[i]][oldest_y + spatern_y[i]] = tile_wall;
            }
        }
    }
}

void tunel_gen(int from_x, int from_y, int direction){
    int dir_x = 0;
    int dir_y = 0;

    switch(direction){
        case upp:
            dir_y = -1;
            break;
        case down:
            dir_y = 1;
            break;
        case left:
            dir_x = -1;
            break;
        case right:
            dir_x = 1;
            break;
    }
    while(map1[from_x][from_y] != tile_floor){
        map1[from_x][from_y] = tile_floor;
        from_x += dir_x;
        from_y += dir_y;
    }

}

void cave_gen(int acces){
    //initial sead
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            int temp = (rand() % 100) + 1;
            if(temp >= 70){
                map1[x][y] = tile_rock;
            }else{
                map1[x][y] = tile_floor;
            }
        }
    }

    //seluar atatmeta
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
                    map2[x][y] = tile_rock;
                }else if(neighbors == 3){
                    map2[x][y] = map1[x][y];
                }else{
                    map2[x][y] = tile_floor;
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

    //remove too smale
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

    //exit tunels
    if(acces & 1 << 0){
        int exit = rand() % (height - 2) + 1;
        tunel_gen(0, exit, right);
        map1[0][exit] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[2] = 0;
        world_map[world_pos_x][world_pos_y].exits_y[2] = exit;
    }
    if(acces & 1 << 1){
        int exit = rand() % (height  - 2) + 1;
        tunel_gen(width - 1, exit, left);
        map1[width - 1][exit] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[3] = width - 1;
        world_map[world_pos_x][world_pos_y].exits_y[3] = exit;
    }
    if(acces & 1 << 2){
        int exit = rand() % (width - 2) + 1;
        tunel_gen(exit, 0, down);
        map1[exit][0] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[0] = exit;
        world_map[world_pos_x][world_pos_y].exits_y[0] = 0;
    }
    if(acces & 1 << 3){
        int exit = rand() % (width - 2) + 1;
        tunel_gen(exit, height - 1, upp);
        map1[exit][height - 1] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[1] = exit;
        world_map[world_pos_x][world_pos_y].exits_y[1] = height - 1;
    }

    add_hardrock();

}

void dungeon_gen(int acces){
    //room generation

    for(int x = 0; x <width + 1; x++){
        for(int y = 0; y <height + 1 ; y++){
            map1[x][y] = 1;
        }
    }

    int max_rooms = 10;


    int rooms_x[max_rooms];
    int rooms_y[max_rooms];
    int rooms = 0;

    int attempts = 0;
    while(rooms < max_rooms){
        int room_width = (rand() % 11) + 5;
        int room_height = (rand() % 11) + 5;
        int room_x = (rand() % width - 1) + 1;
        int room_y = (rand() % height - 1) + 1;

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
                map1[room_x + x][room_y + y] = tile_floor;
                if(x == -1 || x == room_width || y == -1 || y == room_height){
                    map1[room_x + x][room_y + y] = tile_wall;
                }
            }
        }
        rooms++;
        attempts = 0;
    }

    //coridor generation

    for(int i = 0; i < rooms - 1; i++){
        int room1 = rand() % rooms;
        int room2;
        while((room2 = rand() % rooms) == room1);

        coredor_gen(rooms_x[room1], rooms_y[room1], rooms_x[room2], rooms_y[room2]);
    }

    while(1){
        int areas = 0;
        for(int i = 0; i < rooms; i++){
            if(map2[rooms_x[i]][rooms_y[i]] == 0){
                areas++;
                flood_search(rooms_x[i], rooms_y[i], areas);
            }
        }

        if(areas == 1){
            break;
        }

        int room1 = rand() % rooms;
        int room2;
        while((room2 = rand() % rooms) == room1);

        if(map2[rooms_x[room1]][rooms_y[room1]] != map2[rooms_x[room2]][rooms_y[room2]]){
            coredor_gen(rooms_x[room1], rooms_y[room1], rooms_x[room2], rooms_y[room2]);
        }

        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){
                map2[x][y] = 0;
            }
        }
    }

    //exit coredor gen
    if(acces & 1 << 0){
        int exit = rand() % width;
        int furthest = 0;
        for(int i = 1; i < rooms; i++){
            if(rooms_x[i] < rooms_x[furthest]){
                furthest = i;
            }
        }
        coredor_gen(0, exit, rooms_x[furthest], rooms_y[furthest]);
        map1[0][exit] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[2] = 0;
        world_map[world_pos_x][world_pos_y].exits_y[2] = exit;
    }
    if(acces & 1 << 1){
        int exit = rand() % width;
        int furthest = 0;
        for(int i = 1; i < rooms; i++){
            if(rooms_x[i] > rooms_x[furthest]){
                furthest = i;
            }
        }
        coredor_gen(width - 1, exit, rooms_x[furthest], rooms_y[furthest]);
        map1[width - 1][exit] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[3] = width - 1;
        world_map[world_pos_x][world_pos_y].exits_y[3] = exit;
    }
    if(acces & 1 << 2){
        int exit = rand() % width;
        int furthest = 0;
        for(int i = 1; i < rooms; i++){
            if(rooms_y[i] < rooms_y[furthest]){
                furthest = i;
            }
        }
        coredor_gen(rooms_x[furthest], rooms_y[furthest], exit, 0);
        map1[exit][0] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[0] = exit;
        world_map[world_pos_x][world_pos_y].exits_y[0] = 0;

    }
    if(acces & 1 << 3){
        int exit = rand() % width;
        int furthest = 0;
        for(int i = 1; i < rooms; i++){
            if(rooms_y[i] > rooms_y[furthest]){
                furthest = i;
            }
        }
        coredor_gen(rooms_x[furthest], rooms_y[furthest], exit, height -1);
        map1[exit][height - 1] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[1] = exit;
        world_map[world_pos_x][world_pos_y].exits_y[1] = height - 1;
    }

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            if(map1[x][y] == tile_door){
                if( (map1[x + spatern_x[0]][y + spatern_y[0]] == tile_wall && map1[x + spatern_x[1]][y + spatern_y[1]] == tile_wall) || 
                    (map1[x + spatern_x[2]][y + spatern_y[2]] == tile_wall && map1[x + spatern_x[3]][y + spatern_y[3]] == tile_wall)
                ){
                    map1[x][y] = tile_door;
                }else{
                    map1[x][y] = tile_floor;
                }
            }
        }
    }
    add_hardrock();

}

void map_gen(){
    if(world_map[world_pos_x][world_pos_y].sead == 0){
        world_map[world_pos_x][world_pos_y].sead = time(NULL);
    }
    srand(world_map[world_pos_x][world_pos_y].sead);
    if(rand() % 2){
        dungeon_gen(world_map[world_pos_x][world_pos_y].conection);
    }else{
        cave_gen(world_map[world_pos_x][world_pos_y].conection);
    }
}

//player movment and map render
void plase_player(){
    player.x = rand() % width;
    player.y = rand() % height;
    while(map1[player.x][player.y] != 0){
        player.x = rand() % width;
        player.y = rand() % height;    
    }
}

int move_player(){
    //map movment
    int uppdate = 0;
    char c;
    int olde_x = player.x;
    int olde_y = player.y;
    c = getchar();
    printf("\x1b[F \b");
    switch(c){
        case 'w':
        case 'W':
            if( map1[player.x][player.y - 1] == tile_floor ||
                map1[player.x][player.y - 1] == tile_door  ||
                map1[player.x][player.y - 1] == tile_exit
            ){
                player.y--;
                uppdate = 1;
            }
            break;
        
        case 's':
        case 'S':
            if( map1[player.x][player.y + 1] == tile_floor ||
                map1[player.x][player.y + 1] == tile_door  ||
                map1[player.x][player.y + 1] == tile_exit
            ){
                player.y++;
                uppdate = 1;
            }
            break;
        
        case 'a':
        case 'A':
            if( map1[player.x - 1][player.y] == tile_floor ||
                map1[player.x - 1][player.y] == tile_door  ||
                map1[player.x - 1][player.y] == tile_exit
            ){
                player.x--;
                uppdate = 1;
            }
            break;
        
        case 'd':
        case 'D':
            if( map1[player.x + 1][player.y] == tile_floor ||
                map1[player.x + 1][player.y] == tile_door  ||
                map1[player.x + 1][player.y] == tile_exit
            ){
                player.x++;
                uppdate = 1;
            }
            break;
        case 'e':
        case 'E':
            return -1;
    }

    //world movment
    if(map1[player.x][player.y] == tile_exit){
        int dir;
        if(player.x == 0){
            world_pos_x--;
            dir = 3;
        }else if(player.x == width - 1){
            world_pos_x++;
            dir = 2;
        }else if(player.y == 0){
            world_pos_y--;
            dir = 1;
        }else if(player.y == height - 1){
            world_pos_y++;
            dir = 0;
        }
        map_gen();
        player.x = world_map[world_pos_x][world_pos_y].exits_x[dir];
        player.y = world_map[world_pos_x][world_pos_y].exits_y[dir];
    }

    return uppdate;
}

void draw_wall(int x, int y){
    int type = 0;

    for(int i = 0; i < search_cross; i++){
        if(map1[x + spatern_x[i]][y + spatern_y[i]] == tile_wall || map1[x + spatern_x[i]][y + spatern_y[i]] == tile_door){
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

void draw_map(){
    printf("       ");
    for(int x = 0; x < width; x++){
        printf("%2d ", x);
    }
    printf("\n");

    for(int y = 0; y < height; y++){
        printf("y = %2d:", y);
        for(int x = 0; x < width; x++){
            if(x == player.x && y == player.y){
                printf(" \x1b[32m@\x1b[0m ");
                continue;
            }
            switch(map1[x][y]){
                case tile_floor:
                    printf("   ");
                    break;
                case tile_rock:
                    printf(" # ");
                    break;
                case tile_hardrock:
                    printf(" & ");
                    break;
                case tile_wall:
                    draw_wall(x, y);
                    break;
                case tile_door:
                    printf(" D ");
                    break;
                case tile_exit:
                    printf(" E ");
                    break;
                default:
                    printf("???");
            }

        }
        printf("\n");
    }
    // printf("\nworld pos: %d, %d\n", world_pos_x, world_pos_y);
    // for(int y = 0; y < world_height; y++){
    //     printf("y: %2d ", y);
    //     for(int x = 0; x < world_width; x++){

    //         if(x == world_pos_x && y == world_pos_y){
    //             printf("\x1b[31m");
    //         }

    //         wprintf(L"%lc", wallchar[world_map[x][y].conection]);
    //         if(world_map[x][y].conection & 1 << 1 && world_map[x + 1][y].conection & 1){
    //             wprintf(L"%lc", wallchar[1]);
    //         }else{
    //             printf(" ");
    //         }
    //         printf("\x1b[0m");

    //     }
    //     printf("\n");
    // }
}


int main(){
    printf("\x1b[?1049h");

    static struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    setlocale(LC_ALL, "en_US.UTF-8");

    world_map_gen();

    map_gen();

    plase_player();

    printf("\x1b[f");
    draw_map();

    while(1){
        printf("\x1b[f");
        int action = move_player();
        if(action == -1){
            break;
        }else if(action){
            draw_map();
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    printf("\x1b[?1049l");

    return 0;
}
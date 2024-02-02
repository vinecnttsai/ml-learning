#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <iomanip>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;
int dir[5][2]={{1,0},{-1,0},{0,1},{0,-1},{0,0}},fail_count,keyboard=4,percent=10,g=1;
const int batch=1,window_size=1500,map_size=38,size_rectangle=window_size/(map_size+2),max_step=(map_size+2)*10;
int neuro_dir[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,-1},{-1,1},{1,-1}};
float crossover_50[3]={0.2,0.1,0.0};
RenderWindow window(VideoMode(window_size, window_size), "snake game");
class _map
{
    public:
        int id[batch];
        void initialization();
};
void _map::initialization()
{
    for(int i=0;i<batch;i++)id[i]=0;
}
_map map_[map_size][map_size];
bool check_border(int a,int b)
{
    if(a<0||b<0||a>=map_size||b>=map_size)return false;
    else return true;
}



class head_coordinate
{
    public :
        head_coordinate();
        void initialization(int l);
        bool check();
        int posa,posb,fail,fruit_a,fruit_b,step,select,lebal,walk_step,length;
        float fitness,x[input],mutation;
};
head_coordinate head[batch];
head_coordinate::head_coordinate()
{
    posa=map_size/2;
    posb=map_size/2;
    select=0;
    fail=0;
    step=0;
    mutation=rand()%1000/1000.0;
    walk_step=max_step;
}
void head_coordinate::initialization(int l)
{
    posa=map_size/2;
    posb=map_size/2;
    select=0;
    fail=0;
    step=0;
    mutation=rand()%1000/1000.0;
    lebal=l;
    walk_step=max_step;
}
void pick(int num,int time)
{
    int i,k;
    for(int z=0;z<time;z++)
    {
        i=rand()%map_size;
        k=rand()%map_size;
        while(map_[i][k].id[num]!=0)
        {
            i=rand()%map_size;
            k=rand()%map_size;
        }
        map_[i][k].id[num]=2;
        head[num].fruit_a=i;
        head[num].fruit_b=k;
    }
}



class snake
{
    public :
        snake(int prev_dir,int prev_a,int prev_b,int l,int n);
        int direction,posa,posb,lebal,num,prev_dir;
        RectangleShape r;
        void update(int direction_);
        int caculate();
        ~snake(){};
};
snake::snake(int prev_dir,int prev_a,int prev_b,int l,int n)
{
    posa=prev_a+-1*dir[prev_dir][0];
    posb=prev_b+-1*dir[prev_dir][1];
           
    lebal=l;
    num=n;
    r.setFillColor(Color::Black);
    r.setSize(Vector2f(size_rectangle,size_rectangle));
}
void snake::update(int direction_)
{
   
    prev_dir=direction;
    direction=direction_;
    posa+=dir[direction][0];
    posb+=dir[direction][1];

    if(check_border(posa, posb))
    {
        r.setPosition((posa+1)*size_rectangle,(posb+1)*size_rectangle);
        window.draw(r);
    }
}
vector<snake> v[batch];
void draw_map()
{
    RectangleShape r2,r3;
    r2.setFillColor(Color::Red);
    r2.setSize(Vector2f(size_rectangle,size_rectangle));
    for(int i=0;i<batch;i++)
    {
        if(!head[i].fail)
        {
            r2.setPosition((head[i].fruit_a+1)*size_rectangle,(head[i].fruit_b+1)*size_rectangle);
            window.draw(r2);
        }
    }
   
   
    r3.setFillColor(Color::Green);
    r3.setSize(Vector2f(size_rectangle,window_size));
    r3.setPosition(0,0);
    window.draw(r3);
    r3.setPosition(window_size-size_rectangle,0);
    window.draw(r3);
   
    r3.setSize(Vector2f(window_size,size_rectangle));
    r3.setPosition(0,0);
    window.draw(r3);
    r3.setPosition(0,window_size-size_rectangle);
    window.draw(r3);
}
void create_snake(int i)
{
    snake *n=new snake(v[i][v[i].size()-1].direction,v[i][v[i].size()-1].posa,v[i][v[i].size()-1].posb,i,v[i].size());
    v[i].push_back(*n);
}
void initialization()
{
    int i,k;
    fail_count=0;
    for(i=0;i<batch;i++)
    {
        while(v[i].size()!=1)v[i].pop_back();
        //head[i].initialization(i);
        v[i][0].posa=head[i].posa;
        v[i][0].posb=head[i].posb;
        v[i][0].direction=4;
        keyboard=4;
        //for(k=0;k<2;k++)create_snake(i);
    }
    for(i=0;i<map_size;i++)
    {
        for(k=0;k<map_size;k++)
        {
            map_[i][k].initialization();
        }
    }
    for(i=0;i<batch;i++)pick(i,1);

}

void fail(int i,int g)
{
    head[i].fail=1;
    /*
    int score=v[i].size();
    head[i].length=score;
   
    head[i].fitness=head[i].step*0.01+pow(score,3);
    */
    fail_count++;
}

int main()
{
    int i,k;
    fail_count=0;
    srand(time(NULL));
   
    for(i=0;i<map_size;i++)
    {
        for(k=0;k<map_size;k++)
        {
            map_[i][k].initialization();
        }
    }
    for(i=0;i<batch;i++)
    {
        snake *n=new snake(4,head[i].posa,head[i].posb,i,v[i].size());
        v[i].push_back(*n);
        //for(k=0;k<2;k++)create_snake(i);
        map_[head[i].posa][head[i].posb].id[i]=1;
        head[i].lebal=i;
        pick(i, 1);
    }//放於初始化
    
    //cout<<setw(16)<<"FirstFitness"<<setw(16)<<"FirstLength"<<setw(16)<<"Top100Fitness"<<setw(16)<<"AverFitness"<<setw(16)<<"Top100Length"<<setw(16)<<"AverLength"<<setw(16)<<"generation"<<endl;
    
    while (window.isOpen())
    {
        Event event;
   
        while (window.pollEvent(event))
        {
            if (event.type==sf::Event::Closed)
            {
                window.close();
            }
        }
       
        if(fail_count==batch)
        {
        //重啟
            //genetic_algorithm();
            /*
            float aver=0.0,aver_whole=0.0,aver_l=0.0,aver_l_w=0.0;
            for(i=0;i<batch;i++)
            {
                if(i<100)
                {
                    aver+=head[i].fitness;
                    aver_l+=head[i].length;
                }
                aver_whole+=head[i].fitness;
                aver_l_w+=head[i].length;
            }
            aver/=100;
            aver_whole/=batch;
            aver_l/=100;
            aver_l_w/=batch;
            */
            //cout<<setw(16)<<head[0].fitness<<setw(16)<<head[0].length<<setw(16)<<aver<<setw(16)<<aver_whole<<setw(16)<<aver_l<<setw(16)<<aver_l_w<<setw(16)<<g<<endl;
            initialization();
            g++;
        }

       
        usleep(30000);
   
        window.clear(Color::White);
        
        if(Keyboard::isKeyPressed(Keyboard::W))keyboard=3;
        else if(Keyboard::isKeyPressed(Keyboard::S))keyboard=2;
        else if(Keyboard::isKeyPressed(Keyboard::A))keyboard=1;
        else if(Keyboard::isKeyPressed(Keyboard::D))keyboard=0;
        
        draw_map();
       
        for(i=0;i<batch;i++)
        {
            if(!head[i].fail)
            {
                //keyboard=head[i].calculate();
               
                if(map_[v[i][0].posa][v[i][0].posb].id[i]==2)
                {
                    create_snake(i);
                    pick(i,1);
                    head[i].walk_step+=200;
                }
               
                if(dir[v[i][0].direction][0]*-1==dir[keyboard][0]&&dir[v[i][0].direction][1]*-1==dir[keyboard][1])
                {
                    keyboard=v[i][0].direction;
                }
               
                map_[v[i][0].posa][v[i][0].posb].id[i]=0;
                v[i][0].update(keyboard);
                head[i].posa=v[i][0].posa;
                head[i].posb=v[i][0].posb;
                head[i].step++;
                head[i].walk_step--;
                if(head[i].walk_step>max_step)head[i].walk_step=max_step;
                
                if(!check_border(head[i].posa, head[i].posb)||head[i].walk_step<0)
                {
                    fail(i,g);
                    continue;
                }
                else if(map_[v[i][0].posa][v[i][0].posb].id[i]==1)
                {
                    fail(i,g);
                    continue;
                }
               
                if(map_[v[i][0].posa][v[i][0].posb].id[i]!=2)map_[v[i][0].posa][v[i][0].posb].id[i]=1;
            }
        }
        for(k=0;k<batch;k++)
        {
            if(!head[k].fail)
            {
                for(i=1;i<v[k].size();i++)
                {
                    if(check_border(v[k][i].posa, v[k][i].posb))map_[v[k][i].posa][v[k][i].posb].id[k]=0;
                    v[k][i].update(v[k][i-1].prev_dir);
                    if(check_border(v[k][i].posa, v[k][i].posb))map_[v[k][i].posa][v[k][i].posb].id[k]=1;
                }
            }
        }
   
        window.display();
    }
    

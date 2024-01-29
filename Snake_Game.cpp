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
const int batch=2000,window_size=1500,map_size=38,size_rectangle=window_size/(map_size+2),max_step=(map_size+2)*10;
int neuro_dir[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,-1},{-1,1},{1,-1}};
float crossover_50[3]={0.2,0.1,0.0};
RenderWindow window(VideoMode(window_size, window_size), "snake game");
const int layer_num=2,input=30;
int neuron[layer_num+2]={input,16,12,4};
float random_()
{
    if(rand()%2==0)
    {
        return -1.0*(rand()%400000/100000.0);
    }
    else
    {
        return (rand()%400000/100000.0);
    }
}
float sigmoid(float x)
{
    return (1 / (1 + exp(-x)));
}
float relu(float x)
{
    return (x>0)?x:0;
}
class layer
{
    public:
        vector<double> node,bias,weight,mutation,mutation_b;
        int i;
    ~layer(){};
};
class neural_network
{
    public:
        int i,k;
        neural_network();
        layer n[layer_num+2];
        void update(float x[]);
    ~neural_network(){};
};
neural_network::neural_network()
{
    for(i=0;i<layer_num+2;i++)
    {
        for(k=0;k<neuron[i];k++)
        {
            n[i].node.push_back(0.0);
        }
        if(i<layer_num+1)
        {
            for(k=0;k<neuron[i]*neuron[i+1];k++)
            {
                n[i].weight.push_back(random_());
                n[i].mutation.push_back(rand()%100/100.0);
            }
            for(k=0;k<neuron[i+1];k++)
            {
                n[i+1].bias.push_back(random_());
                n[i+1].mutation_b.push_back(rand()%100/100.0);
            }
        }
    }
}


void neural_network::update(float x[])
{
    for(i=0;i<neuron[0];i++)n[0].node[i]=x[i];
   
    for(i=0;i<layer_num+1;i++)
    {
        for(k=0;k<neuron[i]*neuron[i+1];k++)
        {
            n[i+1].node[k%neuron[i+1]]+=n[i].node[k/neuron[i+1]]*n[i].weight[k];
        }
        for(k=0;k<neuron[i+1];k++)
        {
            n[i+1].node[k]=tanh(n[i+1].node[k]+n[i+1].bias[k]);
        }
    }
}


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
        int calculate();
        bool check();
        neural_network network;
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
int head_coordinate::calculate()
{
    float aver=0,sd=0,temp=0,temp2=0;
    int a=posa,b=posb;
    bool flag;
   
    for(int i=0;i<3;i++)
    {
        for(int k=0;k<8;k++)
        {
            a=posa;
            b=posb;
            flag=false;
            if(i==0)
            {
                while(true)
                {
                    if(!check_border(a, b))
                    {
                        if(k>=4)temp*=sqrt(2);
                        flag=true;
                        break;
                    }
                    a+=neuro_dir[k][0];
                    b+=neuro_dir[k][1];
                    temp+=1.0;
                }
            }
            else if(i>0)
            {
                while(check_border(a, b))
                {
                   
                    if((map_[a][b].id[lebal]==i)&&(a!=posa||b!=posb))
                    {
                        if(k>=4)temp*=sqrt(2);
                        flag=true;
                        break;
                    }
                    a+=neuro_dir[k][0];
                    b+=neuro_dir[k][1];
                    temp+=1.0;
                }
            }
            if(!flag)temp=0;
            //else temp=1;
            x[8*i+k]=temp;
            temp=0;
        }
    }
    x[24]=posa;
    x[25]=posb;
    x[26]=fruit_a;
    x[27]=fruit_b;
    x[28]=fruit_a-posa;
    x[29]=fruit_b-posb;
    /*
    for(int i=0;i<input;i++)
    {
        if(x[i])
        {
            aver+=x[i];
            temp2++;
        }
    }
    aver/=float(temp2);
    for(int i=0;i<input;i++)
    {
        if(x[i])sd+=pow(x[i]-aver,2);
    }
    sd/=float(temp2);
    sd=sqrt(sd);
    for(int i=0;i<input;i++)
    {
        if(x[i])x[i]=(x[i]-aver)/sd;
    }//標準化
    */
    for(int i=0;i<input;i++)
    {
        if(x[i])x[i]=1.0/x[i];
    }
   
   
    network.update(x);
   
    float max_=network.n[layer_num+1].node[0];
    int node=0;
    for(int i=0;i<4;i++)
    {
        if(max_<network.n[layer_num+1].node[i])
        {
            max_=network.n[layer_num+1].node[i];
            node=i;
        }
    }
    return node;
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
    //找相反方向
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

void crossover(int parent,int child,int time)
{
    int z,x,p=parent;
    while(time--)
    {
        for(z=0;z<layer_num+1;z++)
        {
            for(x=0;x<neuron[z]*neuron[z+1];x++)
            {
                if(rand()%2==0)p--;
                head[child].network.n[z].weight[x]=head[p].network.n[z].weight[x];
                head[child-1].network.n[z].weight[x]=head[p].network.n[z].weight[x];
                p=parent;
               
            }
            for(x=0;x<neuron[z+1];x++)
            {
                if(rand()%2==0)p--;
                head[child].network.n[z+1].bias[x]=head[p].network.n[z+1].bias[x];
                head[child-1].network.n[z+1].bias[x]=head[p].network.n[z+1].bias[x];
                p=parent;
            }
        }
        child-=2;
    }
}
bool cmp(head_coordinate h1,head_coordinate h2)
{
    if(h1.select==h2.select)return h1.fitness>h2.fitness;
    else return h1.select>h2.select;
}
void selection(int selected)
{
    int i,k;
    while(selected!=batch)
    {
        i=rand()%batch;
        k=rand()%batch;
        if((!head[i].select&&!head[k].select)&&i!=k)
        {
            if(i<k)
            {
                head[i].select=2;
                head[k].select=1;
            }
            else
            {
                head[i].select=1;
                head[k].select=2;
            }
            selected+=2;
        }
       
    }
}
void genetic_algorithm()
{
    int i,k,z,x,remain=batch*percent/100.0;
    sort(head,head+batch,cmp);
   
    //保留一定百分比
    for(i=0;i<remain;i++)
    {
        head[i].select=2;
        head[batch-1-i].select=1;
    }
   
    selection(remain*2);
    //selection
   
    sort(head,head+batch,cmp);
    int p_pos=batch/2-1,now=0,c_pos=batch-1;
    while(p_pos>0)
    {
        if(p_pos/(1.0*batch)<crossover_50[now])now++;
        crossover(p_pos, c_pos, pow(2,now));
        p_pos-=2;
        c_pos-=pow(2,now+1);
    }
   
    for(z=remain;z<batch;z++)
    {
        if(head[z].mutation<0.75)
        {
            for(x=0;x<layer_num+1;x++)
            {
                i=rand()%neuron[x];
                k=rand()%neuron[x+1];
                head[z].network.n[x].weight[i]+=0.01*random_();
                head[z].network.n[x+1].bias[k]+=0.01*random_();
            }
        }
    }//mutation
    //mutation2
    /*
    for(z=remain;z<batch;z++)
    {
        for(x=0;x<layer_num+1;x++)
        {
            for(i=0;i<neuron[x]*neuron[x+1];i++)
            {
                if(head[z].network.n[x].mutation[i]<0.5)head[z].network.n[x].weight[i]+=0.01*random_();
                head[z].network.n[x].mutation[i]=rand()%1000/1000.0;
            }
            for(i=0;i<neuron[x+1];i++)
            {
                if(head[z].network.n[x+1].mutation_b[i]<0.5)head[z].network.n[x+1].bias[i]+=0.01*random_();
                head[z].network.n[x+1].mutation_b[i]=rand()%1000/1000.0;
            }
        }
    }
    */
}

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
        head[i].initialization(i);
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
    int score=v[i].size();
    head[i].length=score;
   
    head[i].fitness=head[i].step*0.01+pow(score,3);
   
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
    
    cout<<setw(16)<<"FirstFitness"<<setw(16)<<"FirstLength"<<setw(16)<<"Top100Fitness"<<setw(16)<<"AverFitness"<<setw(16)<<"Top100Length"<<setw(16)<<"AverLength"<<setw(16)<<"generation"<<endl;
    
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
            genetic_algorithm();
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
            cout<<setw(16)<<head[0].fitness<<setw(16)<<head[0].length<<setw(16)<<aver<<setw(16)<<aver_whole<<setw(16)<<aver_l<<setw(16)<<aver_l_w<<setw(16)<<g<<endl;
            initialization();
            g++;
        }

       
        if(g%100==0)usleep(10000);
   
        window.clear(Color::White);
        /*
        if(Keyboard::isKeyPressed(Keyboard::W))keyboard=3;
        else if(Keyboard::isKeyPressed(Keyboard::S))keyboard=2;
        else if(Keyboard::isKeyPressed(Keyboard::A))keyboard=1;
        else if(Keyboard::isKeyPressed(Keyboard::D))keyboard=0;
        */
        draw_map();
       
        for(i=0;i<batch;i++)
        {
            if(!head[i].fail)
            {
                keyboard=head[i].calculate();
               
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
    return 0;
}

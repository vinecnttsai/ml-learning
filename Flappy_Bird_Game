#include <iostream>
#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>
using namespace std;
using namespace sf;
int size=1500,gravity=-10,jump_height=12,speed=5,r=50,wid=100,v_next_dis,v_next,pipe_dis,world_dis,fail_count,batch=28,generation;
const int input=4,pipe_speed=4;
RenderWindow window(VideoMode(size, size), "flappy bird");
const int layer_num=1;
int neuron[layer_num+2]={input,3,1};
float random_()
{
    if(rand()%2==0)
    {
        return -1.0*(rand()%500000/100000.0);
    }
    else
    {
        return (rand()%500000/100000.0);
    }
}
float sigmoid(float x)
{
    return (1 / (1 + exp(-x)));
}
class layer
{
    public:
        vector<double> node,bias,weight;
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
    }
    for(i=0;i<layer_num+2;i++)
    {
        if(i<layer_num+1)
        {
            for(k=0;k<neuron[i]*neuron[i+1];k++)
            {
                n[i].weight.push_back(random_());
            }
        }
        if(i>0)
        {
            for(k=0;k<neuron[i];k++)
            {
                n[i].bias.push_back(random_());
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
        for(k=0;k<neuron[i];k++)
        {
            n[i+1].node[k]=sigmoid(n[i+1].node[k]+n[i+1].bias[k]);
        }
    }
}




class pipe
{
    public:
        RectangleShape r1;
        RectangleShape r2;
        int local_pos,world_pos,label1,height1,height2;
        pipe(int label,int pos1);
        void update();
        ~pipe(){};
};
pipe::pipe(int label,int pos1)
{
    r1.setFillColor(Color::Green);
    r2.setFillColor(Color::Green);
    world_pos=pos1;
    local_pos=size;
    label1=label;
    height1=100*(10-label-2);
    height2=100*(label+2);
    r1.setSize(Vector2f(wid,height1));
    r2.setSize(Vector2f(wid,height2));
    r1.setPosition(size,0);
    r2.setPosition(size,size-height2);
   

}
void pipe::update()
{
    if(fail_count!=batch)
    {
        local_pos-=speed;
        if(local_pos>size*3/4)
        {
            height1+=pipe_speed;
            height2-=pipe_speed;
        }
        else if(local_pos<size*3/4&&local_pos>size/4)
        {
            height1-=pipe_speed;
            height2+=pipe_speed;
        }
        r1.move(-speed,0);
        r1.setSize(Vector2f(wid,height1));
        r2.setSize(Vector2f(wid,height2));
        r2.setPosition(local_pos,size-height2);
    }
    window.draw(r1);
    window.draw(r2);
}
vector<pipe> v;





class bird
{
    public:
        CircleShape c;
        int world_pos,height,select;
        float x[input],mutation;
        bool fail;
        Vertex line[2][2];
        neural_network network;
        bird();
        void update();
        ~bird(){};
   
};
bird::bird()
{
    c.setFillColor(Color(rand()%200+56,rand()%200+56,rand()%200+56));
    c.setRadius(r);
    c.setPosition(Vector2f(size/2-r,size/2-r));
    for(int i=0;i<2;i++)
    {
        for(int k=0;k<2;k++)line[i][k].color=Color::Red;
    }
    mutation=rand()%1000/1000.0;
    fail=false;
    select=0;
    world_pos=0;
    height=size/2-r;
}
void bird::update()
{
    if(!fail)
    {
        if(height<0||height>size)
        {
            fail=true;
            fail_count++;
        }
        else if(c.getGlobalBounds().intersects(v[v_next].r1.getGlobalBounds())||c.getGlobalBounds().intersects(v[v_next].r2.getGlobalBounds()))
        {
            fail=true;
            fail_count++;
        }
        if(v_next>0)
        {
            if(c.getGlobalBounds().intersects(v[v_next-1].r1.getGlobalBounds())||c.getGlobalBounds().intersects(v[v_next-1].r2.getGlobalBounds()))
            {
                fail=true;
                fail_count++;
            }
        }
        
        x[0]=v[v_next].world_pos-world_pos;//水平距離
        x[1]=height-v[v_next].height1;//上方距離
        x[2]=speed;//速度
        x[3]=size-v[v_next].height2-height;
        float aver=0,sd=0;
        for(int i=0;i<input;i++)
        {
            aver+=x[i];
        }
        aver/=float(input);
        for(int i=0;i<input;i++)
        {
            sd+=pow(x[i]-aver,2);
        }
        sd/=float(input);
        sd=sqrt(sd);
        for(int i=0;i<input;i++)
        {
            x[i]=(x[i]-aver)/sd;
        }//標準化
       
        network.update(x);
        world_pos+=speed;
        if(network.n[layer_num+1].node[0]>0.5)
        {
            height-=jump_height;
        }
        else
        {
            height-=gravity;
        }
        c.setPosition(Vector2f(size/2-r,height));
        
       
        line[0][0].position=Vector2f(size/2, height);
        line[1][0].position=Vector2f(size/2, height);
        line[0][1].position=Vector2f(v[v_next].local_pos, v[v_next].height1);
        line[1][1].position=Vector2f(v[v_next].local_pos, size-v[v_next].height2);
       
        window.draw(c);
        window.draw(line[0],2,Lines);
        window.draw(line[1],2,Lines);

    }
   
}
bird *b=new bird[batch];

void print_weight()
{
    for(int x=0;x<batch;x++)
    {
        for(int y=0;y<layer_num+1;y++)
        {
            for(int z=0;z<neuron[y]*neuron[y+1];z++)
            {
                cout<<b[x].network.n[y].weight[z]<<endl;
            }
        }
        cout<<endl;
    }
}

bool cmp(bird b1,bird b2)
{
    return b1.world_pos>b2.world_pos;
}
bool cmp2(bird b1,bird b2)
{
    if(b1.select==b2.select)
    {
        return b1.world_pos>b2.world_pos;
    }
    else
    {
        return b1.select>b2.select;
    }
}
void crossover_1(int i,int k)
{
    int z,x;
    for(z=0;z<layer_num+1;z++)
    {
        if(z%2==0)
        {
            for(x=0;x<neuron[z]*neuron[z+1];x++)
            {
                swap(b[i].network.n[z].weight[x],b[k].network.n[z].weight[x]);
            }
            for(x=0;x<neuron[z+1];x++)
            {
                swap(b[i].network.n[z+1].bias[x],b[k].network.n[z+1].bias[x]);
            }
        }
    }
}
void crossover_2(int i,int k)
{
    int z,x;
    for(z=0;z<layer_num+1;z++)
    {
        for(x=0;x<neuron[z]*neuron[z+1];x++)
        {
            b[i+batch/2].network.n[z].weight[x]=b[i].network.n[z].weight[x];
            b[k+batch/2].network.n[z].weight[x]=b[k].network.n[z].weight[x];
        }
        for(x=0;x<neuron[z+1];x++)
        {
            b[i+batch/2].network.n[z+1].bias[x]=b[i].network.n[z+1].bias[x];
            b[k+batch/2].network.n[z+1].bias[x]=b[k].network.n[z+1].bias[x];
        }
    }
}

void genetic_algorithm()
{
    int i,k,z,select=2;
    sort(b,b+batch,cmp);
    b[0].select=2;
    b[1].select=2;
   
    while(select!=batch)
    {
        i=rand()%batch;
        k=rand()%batch;
        if(b[i].select==0&&b[k].select==0)
        {
            if(b[i].world_pos>b[k].world_pos)
            {
                b[i].select=2;
                b[k].select=1;
            }
            else
            {
                b[i].select=1;
                b[k].select=2;
            }
            select+=2;
        }
       
    }//selection
   
    sort(b,b+batch,cmp2);
    i=0;
    k=1;
    while(k<batch/2)
    {
        crossover_1(i,k);
        crossover_2(i,k);
        i+=2;
        k+=2;
    }//crossover
    crossover_1(0,1);
   
   
    for(z=0;z<batch;z++)
    {
        if(b[z].mutation<0.6)
        {
            i=rand()%neuron[0];
            k=rand()%neuron[1];
            b[z].network.n[0].weight[i]=random_();
            b[z].network.n[1].weight[k]=random_();
            b[z].network.n[1].bias[k]=random_();
            i=rand()%neuron[2];
            b[z].network.n[2].bias[i]=random_();
           
        }
    }//mutation
   
}



void initialization()
{
    fail_count=0;
    v_next_dis=size/2-r+wid;
    v_next=0;
    world_dis=0;
    for(int z=0;z<batch;z++)
    {
        b[z].world_pos=0;
        b[z].c.setPosition(Vector2f(size/2-r,size/2-r));
        b[z].height=size/2-r;
        b[z].fail=false;
        b[z].mutation=rand()%1000/1000.0;
        b[z].select=0;
    }
    v.clear();
}
int main()
{
    srand(time(NULL));
    window.setFramerateLimit(60);
    v_next_dis=size/2-r+wid;
    v_next=0;
    pipe_dis=450;
    generation=0;
    fail_count=0;
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                   window.close();
            }
        }
        window.clear(Color::White);
        //this_thread::sleep_for(std::chrono::milliseconds(10));
       
        if(world_dis%pipe_dis==0)
        {
            pipe *p=new pipe(rand()%8,int(world_dis/pipe_dis)*pipe_dis+size/2-r);
            v.push_back(*p);
        }
        if(world_dis>v_next_dis)
        {
            v_next+=1;
            v_next_dis+=pipe_dis;
        }
        for(vector<pipe>::iterator i=v.begin();i!=v.end();i++)
        {
            i->update();
            if(i->local_pos<0)
            {
                v.erase(i);
                v_next-=1;
            }
        }
       
        for(int z=0;z<batch;z++)
        {
            b[z].update();
        }
        
       
        if(fail_count!=batch)world_dis+=speed;
        else if(fail_count==batch)
        {
            genetic_algorithm();
            generation++;
            cout<<"generation: "<<generation<<endl;
            /*
             selection
             crossover
             mutation
             */
            initialization();
        }
       
       
        window.display();
    }

return 0;
}

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
float crossover_50[3]={0.2,0.1,0.0};//交配時前50%所生子代佔比
RenderWindow window(VideoMode(window_size, window_size), "snake game");
const int layer_num=2,input=30;
int neuron[layer_num+2]={input,16,12,4};
float random_()
{
    if(rand()%2==0)
    {
        return -1.0*(rand()%4000/1000.0);
    }
    else
    {
        return (rand()%4000/1000.0);
    }
}//介於4與-4的隨機函數
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
};//節點中參數、參數突變率
class neural_network
{
    public:
        int i,k;
        neural_network();
        layer n[layer_num+2];//層
        void update(float x[]);
    ~neural_network(){};
};//神經網路架構
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
}//初始化

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
}//神經元更新weight、bias


class _map
{
    public:
        int id[batch];
        void initialization();
};//地圖類別
void _map::initialization()
{
    for(int i=0;i<batch;i++)id[i]=0;
}//地圖標示記號初始化
_map map_[map_size][map_size];
bool check_border(int a,int b)
{
    if(a<0||b<0||a>=map_size||b>=map_size)return false;
    else return true;
}//檢查是否超出邊界



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
};//貪食蛇頭參數
head_coordinate head[batch];
head_coordinate::head_coordinate()
{
    posa=map_size/2;
    posb=map_size/2;//座標
    select=0;//genetic algorithm時的selection標記
    fail=0;//是否失敗
    step=0;//記錄總步數
    mutation=rand()%1000/1000.0;
    walk_step=max_step;//記錄剩餘步數
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
{//輸入數據至神經元中
    float aver=0,sd=0,temp=0,temp2=0;
    int a=posa,b=posb;
    bool flag;//判斷八個方向延伸是否有目標
    
    for(int i=0;i<3;i++)
    {
        for(int k=0;k<8;k++)
        {
            a=posa;
            b=posb;
            flag=false;
            if(i==0)//判斷邊界
            {
                while(true)
                {
                    if(!check_border(a, b))
                    {
                        if(k>=4)temp*=sqrt(2);//斜邊長
                        flag=true;
                        break;
                    }
                    a+=neuro_dir[k][0];
                    b+=neuro_dir[k][1];
                    temp+=1.0;//計算距離
                }
            }
            else if(i>0)//判斷果實、身體其他部分
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
            x[8*i+k]=temp;
            temp=0;
        }
    }
    x[24]=posa;
    x[25]=posb;//頭部座標
    x[26]=fruit_a;
    x[27]=fruit_b;//果實座標
    x[28]=fruit_a-posa;
    x[29]=fruit_b-posb;//果實、頭部座標差
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
    }//取倒數
    
    
    network.update(x);
    
    float max_=network.n[layer_num+1].node[0];
    int node=0;
    for(int i=0;i<4;i++)//選擇最後一層神經元中的最大值
    {
        if(max_<network.n[layer_num+1].node[i])
        {
            max_=network.n[layer_num+1].node[i];
            node=i;
        }
    }
    return node;//回傳最終移動方向
}
void pick(int num,int time)
{//生成新果實座標
    int i,k;
    for(int z=0;z<time;z++)
    {
        i=rand()%map_size;
        k=rand()%map_size;
        while(map_[i][k].id[num]!=0)//確保新座標尚無其他東西
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
};//貪食蛇一格身體
snake::snake(int prev_dir,int prev_a,int prev_b,int l,int n)
{
    //找前一節移動的相反方向已生成新身體得座標
    posa=prev_a+-1*dir[prev_dir][0];
    posb=prev_b+-1*dir[prev_dir][1];
            
    lebal=l;//貪食蛇編號
    num=n;//屬於第幾節
    r.setFillColor(Color::Black);
    r.setSize(Vector2f(size_rectangle,size_rectangle));
}
void snake::update(int direction_)
{
    
    prev_dir=direction;//紀錄上一個移動方向
    direction=direction_;
    posa+=dir[direction][0];
    posb+=dir[direction][1];
    
    if(check_border(posa, posb)&&lebal<50)
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
    }//混合兩組神經元以組合成新的一組神經網路
}
bool cmp(head_coordinate h1,head_coordinate h2)
{
    if(h1.select==h2.select)return h1.fitness>h2.fitness;
    else return h1.select>h2.select;
}//比較函數在選擇時的優先順序
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
                head[i].select=2;//標記被選上
                head[k].select=1;//標記沒有被選上
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
    
    //無條件保留一定百分比有較高fitness的個體且同時從最劣中剔除最後一位
    for(i=0;i<remain;i++)
    {
        head[i].select=2;
        head[batch-1-i].select=1;
    }
   
    selection(remain*2);
    //selection
   
    sort(head,head+batch,cmp);//在重新排序一次
    int p_pos=batch/2-1,c_pos=batch-1,now=0;//親代個體位子、子代個體位子、交配數量倍率紀錄
    while(p_pos>0)
    {
        if(p_pos/(1.0*batch)<crossover_50[now])now++;
        crossover(p_pos, c_pos, pow(2,now));//用交配出新個體替換最後50％
        p_pos-=2;
        c_pos-=pow(2,now+1);
    }//crossover
    
    
    /*
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
    }
    *///mutation1
    for(z=0;z<batch;z++)
    {
        for(x=0;x<layer_num+1;x++)
        {
            for(i=0;i<neuron[x]*neuron[x+1];i++)
            {
                if(head[z].network.n[x].mutation[i]<0.05)head[z].network.n[x].weight[i]+=random_()*0.01;
                head[z].network.n[x].mutation[i]=rand()%100/100.0;//weight的mutation
            }
            for(i=0;i<neuron[x+1];i++)
            {
                if(head[z].network.n[x+1].mutation_b[i]<0.01)head[z].network.n[x+1].bias[i]+=random_()*0.01;
                head[z].network.n[x+1].mutation_b[i]=rand()%100/100.0;//bias的mutation
            }
        }
    }//mutation2
    
}

void draw_map()
{
    RectangleShape r2,r3;
    r2.setFillColor(Color::Red);
    r2.setSize(Vector2f(size_rectangle,size_rectangle));
    for(int i=0;i<batch;i++)
    {
        if(!head[i].fail&&i<50)
        {
            r2.setPosition((head[i].fruit_a+1)*size_rectangle,(head[i].fruit_b+1)*size_rectangle);
            window.draw(r2);
        }
    }
    //顯示果實位置
    
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
    //顯示邊界
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
{//失敗時結算數據
    head[i].fail=1;
    int score=v[i].size();
    head[i].length=score;
    //總長度
    
    head[i].fitness=head[i].step*0.01+pow(score,3);
    //適存度計算
    
    fail_count++;
}
int main()
{
    int i,k;
    fail_count=0;//batch中失敗個數
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
        map_[head[i].posa][head[i].posb].id[i]=1;//標記頭在地圖上的位置
        head[i].lebal=i;
        pick(i, 1);
    }//放於初始化
    
    while (window.isOpen())
    {
        Event event;
    
        while (window.pollEvent(event))
        {
            if (event.type==sf::Event::Closed)
            {
                window.close();
            }
        }//SFMl基本視窗設定
        */
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
            if(g%2)cout<<head[0].length<<setw(16)<<aver_l<<setw(16)<<aver_l_w<<endl;
            //數據輸出
            initialization();
            g++;//generation
        }

        
        if(g>800)usleep(10000);
    
        window.clear(Color::White);
         /*
        if(Keyboard::isKeyPressed(Keyboard::W))keyboard=3;
        else if(Keyboard::isKeyPressed(Keyboard::S))keyboard=2;
        else if(Keyboard::isKeyPressed(Keyboard::A))keyboard=1;
        else if(Keyboard::isKeyPressed(Keyboard::D))keyboard=0;
        //鍵盤控制
        */
        draw_map();
         
        for(i=0;i<batch;i++)
        {
            if(!head[i].fail)
            {//頭的移動更新
                keyboard=head[i].calculate();
                
                if(map_[v[i][0].posa][v[i][0].posb].id[i]==2)//頭碰到果實
                {
                    create_snake(i);
                    pick(i,1);
                    head[i].walk_step+=200;//多額外可走步數
                }
                
                if(dir[v[i][0].direction][0]*-1==dir[keyboard][0]&&dir[v[i][0].direction][1]*-1==dir[keyboard][1])
                {
                    keyboard=v[i][0].direction;
                }//不可選擇與前一步相反的方向
                
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
                }//碰到邊界or可走步數耗盡
                else if(map_[v[i][0].posa][v[i][0].posb].id[i]==1)
                {
                    fail(i,g);
                    continue;
                }//碰到身體
                
                if(map_[v[i][0].posa][v[i][0].posb].id[i]!=2)map_[v[i][0].posa][v[i][0].posb].id[i]=1;//更新地圖標記
            }
        }
        for(k=0;k<batch;k++)
        {
            if(!head[k].fail)
            {
                for(i=1;i<v[k].size();i++)
                {
                    if(check_border(v[k][i].posa, v[k][i].posb))map_[v[k][i].posa][v[k][i].posb].id[k]=0;//舊位子清除標記
                    v[k][i].update(v[k][i-1].prev_dir);
                    if(check_border(v[k][i].posa, v[k][i].posb))map_[v[k][i].posa][v[k][i].posb].id[k]=1;//新位子標記
                }
            }
        }//更新除頭之外身體的移動
    
        window.display();
    }

    return 0;
}

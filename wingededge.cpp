#include "wingededge.h"
#include <QDebug>

#define INF 65536

WingedEdge::WingedEdge(){

}

WingedEdge::WingedEdge(vector<double> p_x, vector<double> p_y){
    int i=0;
    for(;i<p_x.size();i++){
        g_x.push_back(p_x[i]);
        g_y.push_back(p_y[i]);
    }
    num_polygons = i+1;//i+1 is the infinity polygon
    waiting_merge=false;
}

void WingedEdge::constructOnePointVoronoi()
{
    num_vertices = 0;
    num_edges = 0;
    w.push_back(0); //infinity
}

void WingedEdge::constructTwoPointsVoronoi()
{

    //Assert if # of generating points is 2
    if(this->getNumPolygons() != 2){
        qDebug()<<"Number of generating points is not 2. To use this function, only 2 points is available.";
        return;
    }
    double x_1 = this->g_x[0],x_2 = this->g_x[1];
    double y_1 = this->g_y[0],y_2 = this->g_y[1];

    this->w.resize(2);
    this->w[0] = 0;
    this->w[1] = 0;
    this->num_vertices = 3;
    this->num_edges=3;

    //Initializing all edges' arrays in wingededge data structure
    this->changeArraysForEdges(this->num_edges);

    if(fabs(y_1 - y_2)< 1e-8){//Vertical median line
        //x = b
        double b = (x_1+x_2)/2;

        //(x[0],y[0]) = (b,0)
        x.push_back(b);
        y.push_back(0.0);

        //(x[1],y[1]) = (b,600)
        x.push_back(b);
        y.push_back(600.0);

        if(x_2 < x_1){ //(g_x[0],g_y[0]) on left, (g_x[1],g_y[1])  on right
            iter_swap(g_x.begin()+0,g_x.begin()+1);
            iter_swap(g_y.begin()+0,g_y.begin()+1);
        }
    }
    else if(fabs(x_1 - x_2)< 1e-8){//Horizontal median line
        //y = c
        double c = (y_1+y_2)/2;
        //(x_1,y_1) = (0,c)
        x.push_back(0.0);
        y.push_back(c);

        //(x_1,y_1) = (600,c)
        x.push_back(600.0);
        y.push_back(c);

        if(y_2 > y_1){ //(g_x[0],g_y[0]) at the top, (g_x[1],g_y[1]) at the bot
            iter_swap(g_x.begin()+0,g_x.begin()+1);
            iter_swap(g_y.begin()+0,g_y.begin()+1);
        }
    }
    else{
        //y = mx + b
        double m,b;
        findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);

        //x = ny + c
        double n = (y_1-y_2)/(x_2-x_1);
        double c = b*(y_2-y_1)/(x_2-x_1);

        //Find the 2 vertices cross with the edge of scene
        double y_0,y_600,x_0,x_600;

        //x=0
        y_0 = m*0+b;
        //x=600.0
        y_600 = m*600.0+b;
        //y=0
        x_0 =n*0 + c;
        //y=600.0
        x_600 = n*600.0 + c;

        //Put 2 vertices into vector
        if(y_0 >=0 && y_0 <=600){
            x.push_back(0);
            y.push_back(y_0);
        }
        if(y_600 >=0 && y_600 <=600){
            x.push_back(600.0);
            y.push_back(y_600);
        }
        if(x_0 >=0 && x_0 <=600){
            x.push_back(x_0);
            y.push_back(0);
        }
        if(x_600 >=0 && x_600 <=600){
            x.push_back(x_600);
            y.push_back(600.0);
        }

        //(g_x[0],g_y[0]) should be at left side of edge[0], (g_x[1],g_y[1]) should be at right side of edge[0].
        //Use cross product to judge if (g_x[0],g_y[0]) is on the left of edge[0]
        //a = ((x[1]-x[0]), (y[1]-y[0]))
        //b = ((x[1]-g_x[0]), (y[1]-g_y[0]))
        //a x b > 0 => b is on the left side of a
        if((x[1]-x[0])*(x[1]-g_x[0]) - (y[1]-y[0])*(y[1]-g_y[0]) < 0){
            iter_swap(x.begin()+0,x.begin()+1);
            iter_swap(y.begin()+0,y.begin()+1);
        }
    }

    //Set edge[0], edge[0] is the ordinary edge of 2 points from vertex[0]->vertex[1]
    this->configArraysForEdges(0,1,0,0,1,2,1,1,2);

    //Set edge[1], edge[1] is on the outer side of generating_point[0] from edge[0]
    this->configArraysForEdges(1,0,2,0,1,0,2,2,0);

    //Set edge[2], edge[2] is on the outer side of generating_point[1] from edge[0]
    this->configArraysForEdges(2,2,0,0,1,1,0,0,1);
}

void WingedEdge::constructThreePointsVoronoi()
{
    if(this->getNumPolygons() != 3){
        qDebug()<<"Number of generating points is not 3. To use this function, only 3 points is available.";
        return;
    }

    /*依照(x,y) sorts generating points in increasing order*/
    vector<struct g_point> g_p;
    struct g_point tmp_p;
    tmp_p.x = this->g_x[0];
    tmp_p.y = this->g_y[0];
    g_p.push_back(tmp_p);

    tmp_p.x = this->g_x[1];
    tmp_p.y = this->g_y[1];
    g_p.push_back(tmp_p);

    tmp_p.x = this->g_x[2];
    tmp_p.y = this->g_y[2];
    g_p.push_back(tmp_p);

    sort(g_p.begin(),g_p.end(),compare_g_point);
    this->g_x[0] = g_p[0].x;this->g_y[0] = g_p[0].y;
    this->g_x[1] = g_p[1].x;this->g_y[1] = g_p[1].y;
    this->g_x[2] = g_p[2].x;this->g_y[2] = g_p[2].y;

    //Judge three points' positions to construct the voronoi
    if( (int(this->g_y[0])==int(this->g_y[1])) && (int(this->g_y[1])==int(this->g_y[2])) ){
        /*三點共線-水平*/

        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);
        //畫兩條垂直edge
        //First edge : (x[0],y[0])~(x[1],y[1])
        this->x[0] = (this->g_x[0]+this->g_x[1])/2;
        this->y[0] = 0;     this->w[0] = 0;//points at infinity

        this->x[1] = (this->g_x[0]+this->g_x[1])/2;
        this->y[1] = 600;   this->w[1] = 0;//points at infinity

        //Second edge : (x[2],y[2])~(x[3],y[3])
        this->x[2] = (this->g_x[1]+this->g_x[2])/2;
        this->y[2] = 0;     this->w[2] = 0;//points at infinity

        this->x[3] = (this->g_x[1]+this->g_x[2])/2;
        this->y[3] = 600;   this->w[3] = 0;//points at infinity

        this->num_vertices = 4;

        //Setting edges
        this->num_edges=6;//2 oridinary edges, 4 augumented edges
        this->changeArraysForEdges(this->num_edges);

        /***Todo***/
        //Config edges' arrays
        //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
        this->configArraysForEdges(0,1,0,0,1,5,2,2,3);
        this->configArraysForEdges(1,2,1,2,3,4,5,3,4);
        this->configArraysForEdges(2,0,3,0,1,0,5,1,0);
        this->configArraysForEdges(3,1,3,1,3,0,2,4,1);
        this->configArraysForEdges(4,2,3,3,2,1,3,5,1);
        this->configArraysForEdges(5,1,3,2,0,1,4,2,0);

    }
    else if( (int(this->g_x[0])==int(this->g_x[1])) && (int(this->g_x[1])==int(this->g_x[2])) ){
        /*三點共線-垂直*/
        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);
        //畫兩條水平edge
        //First edge : (x[0],y[0])~(x[1],y[1])
        this->x[0] = 0;
        this->y[0] = (this->g_y[0]+this->g_y[1])/2;
        this->w[0] = 0;//points at infinity

        this->x[1] = 600;
        this->y[1] = (this->g_y[0]+this->g_y[1])/2;
        this->w[1] = 0;//points at infinity

        //Second edge : (x[2],y[2])~(x[3],y[3])
        this->x[2] = 0;
        this->y[2] = (this->g_y[1]+this->g_y[2])/2;
        this->w[2] = 0;//points at infinity

        this->x[3] = 600;
        this->y[3] = (this->g_y[1]+this->g_y[2])/2;
        this->w[3] = 0;//points at infinity

        this->num_vertices = 4;

        //Setting edges
        this->num_edges=6;//2 oridinary edges, 4 augumented edges
        this->changeArraysForEdges(this->num_edges);

        /***Todo***/
        //Config edges' arrays
        this->configArraysForEdges(0,0,1,0,1,2,5,3,2);
        this->configArraysForEdges(1,1,2,2,3,5,4,4,3);
        this->configArraysForEdges(2,3,0,0,1,5,0,0,1);
        this->configArraysForEdges(3,3,1,1,3,2,0,1,4);
        this->configArraysForEdges(4,3,2,3,2,3,1,1,5);
        this->configArraysForEdges(5,3,1,2,0,4,1,0,2);

    }
    else if( fabs((this->g_y[1]-this->g_y[0])/(this->g_x[1]-this->g_x[0]) - (this->g_y[2]-this->g_y[1])/(this->g_x[2]-this->g_x[1])) < 1e-8 ){
        /*三點共線-其他*/
        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);

        double x_1,y_1,x_2,y_2;
        x_1 = this->g_x[0];
        y_1 = this->g_y[0];
        x_2 = this->g_x[1];
        y_2 = this->g_y[1];

        //y = mx + b , x = ny + c
        double m,b,n,c;
        this->findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);
        n = 1/m;        c = (-1)*b/m;

        /** Start adding vertices **/

        //Judge where the vertices should be.
        if(m>=0){
            //Line of (x[0],y[0]),(x[1],y[1])
            if(b >= 0 && b<= 600){
                //first vertix is (0,b)
                this->x[0] = 0;
                this->y[0] = b;
                this->w[0] = 0;
            }
            else{
                //first vertix is (c,0)
                this->x[0] = c;
                this->y[0] = 0;
                this->w[0] = 0;
            }

            //x_cross_y_600 is the x-coordinate of the point that intersect with y=600
            double x_cross_y_600 = n*600+c;
            if(x_cross_y_600 >=0 && x_cross_y_600 <=600){
                //second vertex is at the bound of y=600
                this->x[1] = x_cross_y_600;
                this->y[1] = 600;
                this->w[1] = 0;
            }
            else{
                //second vertex is at the bound of x=600
                this->x[1] = 600;
                this->y[1] = m*600+b;
                this->w[1] = 0;
            }

            /******************************************************************/

            //Line of (x[1],y[1]),(x[2],y[2])
            x_1 = this->g_x[1];
            y_1 = this->g_y[1];
            x_2 = this->g_x[2];
            y_2 = this->g_y[2];
            this->findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);
            n = 1/m;        c = (-1)*b/m;

            if(b >= 0 && b<= 600){
                //first vertix is (0,b)
                this->x[2] = 0;
                this->y[2] = b;
                this->w[2] = 0;
            }
            else{
                //first vertix is (c,0)
                this->x[2] = c;
                this->y[2] = 0;
                this->w[2] = 0;
            }

            //x_cross_y_600 is the x-coordinate of the point that intersect with y=600
            x_cross_y_600 = n*600+c;
            if(x_cross_y_600 >=0 && x_cross_y_600 <=600){
                //second vertex is at the bound of y=600
                this->x[3] = x_cross_y_600;
                this->y[3] = 600;
                this->w[3] = 0;
            }
            else{
                //second vertex is at the bound of x=600
                this->x[3] = 600;
                this->y[3] = m*600+b;
                this->w[3] = 0;
            }
            /******************************************************************/

            //Config edges' arrays
            //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
            this->configArraysForEdges(0,1,0,0,1,5,2,2,3);
            this->configArraysForEdges(1,2,1,2,3,4,5,3,4);
            this->configArraysForEdges(2,0,3,0,1,0,5,1,0);
            this->configArraysForEdges(3,1,3,1,3,0,2,4,1);
            this->configArraysForEdges(4,2,3,3,2,1,3,5,1);
            this->configArraysForEdges(5,1,3,2,0,1,4,2,0);

            /******************************************************************/
        }
        else{//m<0
            if(b >= 0 && b<= 600){
                //first vertix is (0,b)
                this->x[0] = 0;
                this->y[0] = b;
                this->w[0] = 0;
            }
            else{
                //first vertix is (n*600+c,600)
                this->x[0] = n*600+c;
                this->y[0] = 600;
                this->w[0] = 0;
            }

            //x_cross_y_0 is the x-coordinate of the point that intersect with y=0
            double x_cross_y_0 = n*0+c;

            if(x_cross_y_0 >0 && x_cross_y_0 <600){
                //second vertex is at the bound of y=0
                this->x[1] = x_cross_y_0;
                this->y[1] = 0;
                this->w[1] = 0;
            }
            else{
                //second vertex is at the bound of x=600
                this->x[1] = 600;
                this->y[1] = m*600+b;
                this->w[1] = 0;
            }

            /******************************************************************/
            //Line of (x[1],y[1]),(x[2],y[2])
            x_1 = this->g_x[1];
            y_1 = this->g_y[1];
            x_2 = this->g_x[2];
            y_2 = this->g_y[2];
            this->findPerpendicularBisector(x_1,y_1,x_2,y_2,m,b);
            n = 1/m;        c = (-1)*b/m;

            if(b >= 0 && b<= 600){
                //first vertix is (0,b)
                this->x[2] = 0;
                this->y[2] = b;
                this->w[2] = 0;
            }
            else{
                //first vertix is (n*600+c,600)
                this->x[2] = n*600+c;
                this->y[2] = 600;
                this->w[2] = 0;
            }

            //x_cross_y_0 is the x-coordinate of the point that intersect with y=0
            x_cross_y_0 = n*0+c;

            if(x_cross_y_0 >0 && x_cross_y_0 <600){
                //second vertex is at the bound of y=0
                this->x[3] = x_cross_y_0;
                this->y[3] = 0;
                this->w[3] = 0;
            }
            else{
                //second vertex is at the bound of x=600
                this->x[3] = 600;
                this->y[3] = m*600+b;
                this->w[3] = 0;
            }
            /******************************************************************/

            //Setting edges
            this->num_edges=6;//2 oridinary edges, 4 augumented edges
            this->changeArraysForEdges(this->num_edges);

            //Config edges' arrays
            //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
            this->configArraysForEdges(0,1,0,1,0,5,2,2,3);
            this->configArraysForEdges(1,2,1,3,2,4,5,3,4);
            this->configArraysForEdges(2,0,3,1,0,0,5,1,0);
            this->configArraysForEdges(3,1,3,0,2,0,2,4,1);
            this->configArraysForEdges(4,2,3,2,3,1,3,5,1);
            this->configArraysForEdges(5,1,3,3,1,1,4,2,0);

            /******************************************************************/
        }
        /** Done add vertices **/

    }
    else if( (int(this->g_x[0]) == int(this->g_x[1])) || (int(this->g_x[1])==int(this->g_x[2])) ){
        /*兩點垂直*/
        //1 oridinary vertex(circumcenter), 3 infinity vertices
        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);

        if(fabs(this->g_x[0] - this->g_x[1]) < 1e-8){
            //2 points on left, 1 point on right
            this->x[0] = 0;
            this->y[0] = (this->g_y[0]+this->g_y[1])/2;
            this->w[0] = 0;

            double m_top,m_bot,b_top,b_bot;
            double n_top,n_bot,c_top,c_bot;

            if(fabs(this->g_y[1] - this->g_y[2]) < 1e-8){
                this->x[1] = (this->g_x[1] + this->g_x[2])/2;
                this->y[1] = 600;
                this->w[1] = 0;

                //Circumcenter
                this->x[3] = (this->g_x[1] + this->g_x[2])/2;
                this->y[3] = (this->g_y[0] + this->g_y[1])/2;
                this->w[3] = 1;
            }
            else{
                this->findPerpendicularBisector(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],m_top,b_top);
                n_top = 1/m_top;    c_top = (-1)*b_top/m_top;
                double x_cross_y_600;
                x_cross_y_600 = n_top*600+c_top;

                //point intersect with upper margin
                if(x_cross_y_600>=0 && x_cross_y_600<=600){
                    this->x[1] = x_cross_y_600;
                    this->y[1] = 600;
                    this->w[1] = 0;
                }
                else if(x_cross_y_600 <0){
                    this->x[1] = 0;
                    this->y[1] = b_top;
                    this->w[1] = 0;
                }
                else{
                    this->x[1] = 600;
                    this->y[1] = m_top*600+b_top;
                    this->w[1] = 0;
                }


                //Circumcenter
                this->x[3] = (b_bot-b_top)/(m_top-m_bot);
                this->y[3] = m_top * (this->y[3]) +b_top;
                this->w[3] = 1;

            }

            if(fabs(this->g_y[0] - this->g_y[2]) < 1e-8){
                this->x[2] = (this->g_x[0] + this->g_x[2])/2;
                this->y[2] = 0;
                this->w[2] = 0;

                //Circumcenter
                this->x[3] = (this->g_x[0] + this->g_x[2])/2;
                this->y[3] = (this->g_y[0] + this->g_y[1])/2;
                this->w[3] = 1;
            }
            else{
                this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[2],this->g_y[2],m_bot,b_bot);
                n_bot = 1/m_bot;    c_bot = (-1)*b_bot/m_bot;

                double x_cross_y_0;
                x_cross_y_0 = n_bot*600+c_bot;

                //points intersect with lower margin
                if(x_cross_y_0>=0 && x_cross_y_0<=600){
                    this->x[2] = x_cross_y_0;
                    this->y[2] = 0;
                    this->w[2] = 0;
                }
                else if(x_cross_y_0 <0){
                    this->x[2] = 0;
                    this->y[2] = b_bot;
                    this->w[2] = 0;
                }
                else{
                    this->x[2] = 600;
                    this->y[2] = b_bot;
                    this->w[2] = 0;
                }


                //Circumcenter
                this->x[3] = (b_bot-b_top)/(m_top-m_bot);
                this->y[3] = m_top * (this->y[3]) +b_top;
                this->w[3] = 1;
            }
            /******************************************************************/

            //Setting edges
            this->num_edges=6;//3 oridinary edges, 3 augumented edges
            this->changeArraysForEdges(this->num_edges);

            //Config edges' arrays
            //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
            this->configArraysForEdges(0,0,1,0,3,5,3,1,2);
            this->configArraysForEdges(1,1,2,1,3,3,4,2,0);
            this->configArraysForEdges(2,2,0,2,3,4,5,0,1);
            this->configArraysForEdges(3,1,3,0,1,0,5,4,1);
            this->configArraysForEdges(4,2,3,1,2,1,3,5,2);
            this->configArraysForEdges(5,0,3,2,0,2,4,3,0);

            /******************************************************************/

        }
        else{
            //2 points on right, 1 point on left
            this->x[0] = 600;
            this->y[0] = (this->g_y[1]+this->g_y[2])/2;
            this->w[0] = 0;

            double m_top,m_bot,b_top,b_bot;
            double n_top,n_bot,c_top,c_bot;

            if(fabs(this->g_y[0] - this->g_y[2]) < 1e-8){
                this->x[1] = (this->g_x[0] + this->g_x[2])/2;
                this->y[1] = 600;
                this->w[1] = 0;
            }
            else{
                this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[2],this->g_y[2],m_top,b_top);
                n_top = 1/m_top;
                c_top = (-1)*b_top/m_top;
                double x_cross_y_600;
                x_cross_y_600 = n_top*600+c_top;

                //point intersect with upper margin
                if(x_cross_y_600>=0 && x_cross_y_600<=600){
                    this->x[1] = x_cross_y_600;
                    this->y[1] = 600;
                    this->w[1] = 0;
                }
                else if(x_cross_y_600 <0){
                    this->x[1] = 0;
                    this->y[1] = b_top;
                    this->w[1] = 0;
                }
                else{
                    this->x[1] = 600;
                    this->y[1] = m_top*600+b_top;
                    this->w[1] = 0;
                }
            }

            if(fabs(this->g_y[0] - this->g_y[1]) < 1e-8){
                this->x[2] = (this->g_x[0] + this->g_x[1])/2;
                this->y[2] = 600;
                this->w[2] = 0;
            }
            else{
                this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[1],this->g_y[1],m_bot,b_bot);
                n_bot = 1/m_bot;    c_bot = (-1)*b_bot/m_bot;

                double x_cross_y_0;
                x_cross_y_0 = n_bot*600+c_bot;

                //points intersect with lower margin
                if(x_cross_y_0>=0 && x_cross_y_0<=600){
                    this->x[2] = x_cross_y_0;
                    this->y[2] = 0;
                    this->w[2] = 0;
                }
                else if(x_cross_y_0 <0){
                    this->x[2] = 0;
                    this->y[2] = b_bot;
                    this->w[2] = 0;
                }
                else{
                    this->x[2] = 600;
                    this->y[2] = b_bot;
                    this->w[2] = 0;
                }

            }
            //Circumcenter
            this->x[3] = (b_bot-b_top)/(m_top-m_bot);
            this->y[3] = m_top*this->x[3]+b_top;
            this->w[3] = 1;

            /******************************************************************/

            //Setting edges
            this->num_edges=6;//3 oridinary edges, 3 augumented edges
            this->changeArraysForEdges(this->num_edges);

            //Config edges' arrays
            //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
            this->configArraysForEdges(0,2,1,0,3,3,5,2,1);
            this->configArraysForEdges(1,0,2,1,3,4,3,0,2);
            this->configArraysForEdges(2,1,0,2,3,5,4,1,0);
            this->configArraysForEdges(3,3,2,0,1,5,0,1,4);
            this->configArraysForEdges(4,3,0,1,2,3,1,2,5);
            this->configArraysForEdges(5,3,1,2,0,4,2,0,3);

            /******************************************************************/
        }
    }
    else{
        /*一般情況*/

        this->w.resize(4);
        this->x.resize(4);this->y.resize(4);

        /******* Divide 3 points as : left -- 1 point , right -- 2 points *******/

        if((this->g_y[2] -this->g_y[0])/(this->g_x[2] -this->g_x[0])< (this->g_y[1] -this->g_y[0])/(this->g_x[1] -this->g_x[0])){
            //Let upper-m be the vertex number 2
            //    lower-m be the vertex number 1
            double tmp = this->g_y[1];
            this->g_y[1] = this->g_y[2];
            this->g_y[2] = tmp;

            tmp = this->g_x[1];
            this->g_x[1] = this->g_x[2];
            this->g_x[2] = tmp;
        }

        double m_top,m_bot,b_top,b_bot;
        double n_top,n_bot,c_top,c_bot;

        //The coordinate of infinity vertex of left & upper-right point 's perpendicular bisector
        if(fabs(this->g_y[0] - this->g_y[2]) < 1e-8){
            this->x[1] = (this->g_x[0] + this->g_x[2])/2;
            this->y[1] = 600;
            this->w[1] = 0;
        }
        else{
            this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[2],this->g_y[2],m_top,b_top);
            n_top = 1/m_top;
            c_top = (-1)*b_top/m_top;
            double x_cross_y_600;
            x_cross_y_600 = n_top*600+c_top;

            //point intersect with upper margin
            if(x_cross_y_600>=0 && x_cross_y_600<=600){
                this->x[1] = x_cross_y_600;
                this->y[1] = 600;
                this->w[1] = 0;
            }
            else if(x_cross_y_600 <0){
                this->x[1] = 0;
                this->y[1] = b_top;
                this->w[1] = 0;
            }
            else{
                this->x[1] = 600;
                this->y[1] = m_top*600+b_top;
                this->w[1] = 0;
            }
        }

        //The coordinate of infinity vertex of left & lower-right point 's perpendicular bisector
        if(fabs(this->g_y[0] - this->g_y[1]) < 1e-8){
            this->x[2] = (this->g_x[0] + this->g_x[1])/2;
            this->y[2] = 600;
            this->w[2] = 0;
        }
        else{
            this->findPerpendicularBisector(this->g_x[0],this->g_y[0],this->g_x[1],this->g_y[1],m_bot,b_bot);
            n_bot = 1/m_bot;    c_bot = (-1)*b_bot/m_bot;

            double x_cross_y_0;
            x_cross_y_0 = c_bot;

            //points intersect with lower margin
            if(x_cross_y_0>=0 && x_cross_y_0<=600){
                this->x[2] = x_cross_y_0;
                this->y[2] = 0;
                this->w[2] = 0;
            }
            else if(x_cross_y_0 <0){
                this->x[2] = 0;
                this->y[2] = b_bot;
                this->w[2] = 0;
            }
            else{
                this->x[2] = 600;
                this->y[2] = 600*m_bot+b_bot;
                this->w[2] = 0;
            }

        }

        //Circumcenter, which is an ordinary vertex
        this->x[3] = (b_bot-b_top)/(m_top-m_bot);
        this->y[3] = m_top*this->x[3]+b_top;
        this->w[3] = 1;


        //The coordinate of infinity vertex of 2 right-most-generating-points' perpendicular bisector
        double m,b,n,c;
        double candidate1_x,candidate1_y,candidate2_x,candidate2_y;

        if(fabs(this->g_y[1]-this->g_y[2]) < 1e-8){
            candidate1_x = (this->g_x[1]+this->g_x[2])/2;
            candidate1_y = 0;
            candidate2_x = (this->g_x[1]+this->g_x[2])/2;
            candidate2_y = 600;
        }
        else{
            this->findPerpendicularBisector(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],m,b);
            n = 1/m;
            c = (-1)*b/m;

            candidate1_x = n*0+c;
            candidate1_y = 0;

            candidate2_x = n*600+c;
            candidate2_y = 600;

        }

        //Determine if the generating point at left side is at different side with circumcenter
        double cross_product_of_left_point = cross_product(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],this->g_x[0],this->g_y[0]);
        double cross_product_of_candidate1 = cross_product(this->g_x[1],this->g_y[1],this->g_x[2],this->g_y[2],candidate1_x,candidate1_y);

        if(cross_product_of_left_point*cross_product_of_candidate1<0){
            //the generating point at left side is at different side with circumcenter
            if(candidate1_x>=0 && candidate1_x<=600){
                this->x[0] = candidate1_x;
                this->y[0] = candidate1_y;
                this->w[0] = 0;
            }
            else if(candidate1_x < 0){
                this->x[0] = 0;
                this->y[0] = m*0+b;
                this->w[0] = 0;
            }
            else{
                this->x[0] = 600;
                this->y[0] = m*600+b;
                this->w[0] = 0;
            }
        }
        else{
            //candidate1 is at different side with circumcenter

            if(candidate2_x>=0 && candidate2_x<=600){
                this->x[0] = candidate2_x;
                this->y[0] = candidate2_y;
                this->w[0] = 0;
            }
            else if(candidate2_x < 0){
                this->x[0] = 0;
                this->y[0] = m*0+b;
                this->w[0] = 0;
            }
            else{
                this->x[0] = 600;
                this->y[0] = m*600+b;
                this->w[0] = 0;
            }
        }

        /******************************************************************/

        //Setting edges
        this->num_edges=6;//3 oridinary edges, 3 augumented edges
        this->changeArraysForEdges(this->num_edges);

        //Config edges' arrays
        //void configArraysForEdges(int edge_num,int rp,int lp,int sv,int ev,int cw_p,int ccw_p,int cw_s,int ccw_s);
        this->configArraysForEdges(0,2,1,0,3,3,5,2,1);
        this->configArraysForEdges(1,0,2,1,3,4,3,0,2);
        this->configArraysForEdges(2,1,0,2,3,5,4,1,0);
        this->configArraysForEdges(3,3,2,0,1,5,0,1,4);
        this->configArraysForEdges(4,3,0,1,2,3,1,2,5);
        this->configArraysForEdges(5,3,1,2,0,4,2,0,3);

        /******************************************************************/
    }

}

double WingedEdge::cross_product(double x_0, double y_0, double x_1, double y_1, double x_2, double y_2)
{
    return (x_1-x_0)*(y_2-y_0) - (x_2-x_0)*(y_1-y_0);
}

void WingedEdge::divide(WingedEdge &W_l, WingedEdge &W_r)
{

    //Divide part
    vector<double> l_x,l_y,r_x,r_y;
    double L ;

    //Step 1 : Find a median line L perpendicular to the X-axis
    //which divides W into W_l and W_r, with equal sizes.
    //Use Prune-and-Search
    L = this->find_k_th_Line(this->get_g_x(),this->g_x.size()/2);

    for(unsigned long i=0;i<this->g_x.size();i++){
        //num_polygons should be same as g_x.size() and g_y.size()
        //Less than m, put in left
        if(this->g_x[i] < L){
            l_x.push_back(g_x[i]);
            l_y.push_back(g_y[i]);
        }
        else if(this->g_x[i] >= L ){
            r_x.push_back(g_x[i]);
            r_y.push_back(g_y[i]);
        }
    }

    W_l = WingedEdge(l_x,l_y);
    W_r = WingedEdge(r_x,r_y);
    qDebug()<<"W_l.getNumPolygons()"<<W_l.getNumPolygons();
    qDebug()<<"W_r.getNumPolygons()"<<W_r.getNumPolygons();

}

void WingedEdge::merge(WingedEdge S_l, WingedEdge S_r)
{
    /******************/

    /* Construct a dividing piece-wise linear
     * hyperplane HP which is the locus of points
     * simultaneously closest to a point in S_l
     * and a point in S_r.
     */

    /* Step 1: Find the convex hulls of SL
     * and SR,denoted as Hull(SL) and Hull(SR),
     * respectively.
     */

    /* Step 2: Find segments PaPb and PcPd which join
     * HULL(SL) and HULL(SR) into a convex hull
     * (Pa and Pc belong to SL and Pb and
     *  Pd belong to SR).
     * Assume that PaPb lies above PcPd.
     * Let x = a, y = b, SG= PxPy and HP = empty
     */


    /* Step 3: Find the perpendicular bisector of SG.
     * Denote it by BS. Let HP = HP∪{BS}.
     * If SG = PcPd, go to Step 5; otherwise, go to Step 4
     */

    /* Step 4: The ray from VD(SL) and VD(SR) which
     * BS first intersects with must be a perpendicular
     * bisector of either PxPz or PyPz for some z.
     * If this ray is the perpendicular bisector of PyPz, then let SG = PxPz ;
     * otherwise, let SG = PzPy. Go to Step 3.
     */

    /* Step 5: Discard the edges of VD(SL) which extend to
     * the right of HP and discard the edges of VD(SR) which
     * extend to the left of HP.
     * The resulting graph is the Voronoi diagram of S = SL ∪ SR
     */

}

double WingedEdge::find_k_th_Line(vector<double> S,unsigned long k)
{
    if(S.size() <= 20){
        //If the size is less than 20, sort it directly
        //O(1)

        sort(S.begin(),S.end());

        if(S.size()%2 == 0){
            return (S[S.size()/2]+S[S.size()/2 - 1])/2;
        }
        else{
            return S[S.size()/2];
        }
    }

    //Step 1: Divide S into n/5 subsets
    //Add some dummy INF points to the last subset
    //if n is not a net multiple of 5
    if(S.size() % 5 != 0){
        //Add some dummy INF points
        //O(1)
        for(unsigned long i=0;i<5-(S.size() % 5);i++){
            S.push_back(INF);
        }
    }

    //Step 2: Sort each subset of elements and add the median into "medians"
    //O(n)
    vector<double> medians;
    for(unsigned long i=0;i<S.size();i+=5){
        vector<double> tmp(S.begin()+i,S.begin()+i+4);
        sort(tmp.begin(),tmp.end());
        medians.push_back(tmp[2]);
    }

    //Step 3: Recursively sort "medians" to find the element p whih is the median of medians
    //T(n/5)
    double p = this->find_k_th_Line(medians, S.size()/2);

    /** Not necessary to do Step 4 and 5 in this project, because p is the answer
     *  But for the convenience of future porting, I finished them. **/
    //Step 4: Partition S into S1, S2 and S3,
    //which contain the elements less than, equal to, and greater than p, respectively.
    //O(n)
    vector<double> S1,S2,S3;

    for(unsigned long i=0;i<S.size();i++){
        if(S[i] < p)
            S1.push_back(S[i]);
        else if(fabs(S[i]-p)<1e-8)
            S2.push_back(S[i]);
        else
            S3.push_back(S[i]);
    }

    //Step 5: Determine where the median located in S1, S2 or S3.
    //        Prune the other 2 and recursively search
    //T(4n/5)

    if(S1.size() >= k){
        return find_k_th_Line(S1,k);
    }
    else if(S1.size()+S2.size() >= k){
        /** In this project, it should return here **/
        return p;
    }
    else{
        return find_k_th_Line(S3,k-S1.size()-S2.size());
    }
}

int WingedEdge::getNumPolygons()
{
    return num_polygons-1;//Except for p_infinity
}

bool WingedEdge::threePointsVertical()
{
    return num_polygons==3 && (int)g_x[0]==(int)g_x[1] && (int)g_x[1]==(int)g_x[2];
}

void WingedEdge::setWaitingMerge(bool i)
{
    this->waiting_merge = i;
}

bool WingedEdge::IsWaitingMerge()
{
    return this->waiting_merge;
}

void WingedEdge::changeArraysForEdges(int resize_size)
{
    this->right_polygon.resize(resize_size);
    this->left_polygon.resize(resize_size);
    this->start_vertex.resize(resize_size);
    this->end_vertex.resize(resize_size);
    this->cw_predecessor.resize(resize_size);
    this->ccw_predecessor.resize(resize_size);
    this->cw_successor.resize(resize_size);
    this->ccw_successor.resize(resize_size);
}

void WingedEdge::configArraysForEdges(int edge_num, int rp, int lp, int sv, int ev, int cw_p, int ccw_p, int cw_s, int ccw_s)
{
    //edge_num is the edge number to be config
    //rp stands for right_polygon
    //lp stands for left_polygon
    //sv stands for start_vertex
    //ev stands for end_vertex
    //cw_p stands for cw_predecessor
    //ccw_p stands for ccw_predecessor
    //cw_s stands for cw_successor
    //ccw_s stands for ccw_successor

    this->right_polygon[edge_num]=rp;
    this->left_polygon[edge_num]=lp;
    this->start_vertex[edge_num]=sv;
    this->end_vertex[edge_num]=ev;
    this->cw_predecessor[edge_num]=cw_p;
    this->ccw_predecessor[edge_num]=ccw_p;
    this->cw_successor[edge_num]=cw_s;
    this->ccw_successor[edge_num]=ccw_s;

}

vector<int> WingedEdge::getOrdinaryEdges()
{
    vector<int> e;
    //Don't return augumented edges but oridinary edges.
    //The augumented edges are adjacent to p_infinity
    for(unsigned long i=0;i<num_edges;i++){
        if(right_polygon[i] == this->getNumPolygons() || left_polygon[i] == this->getNumPolygons())
            continue;
        //put oridinary edge number into e
        e.push_back(i);
    }
    return e;
}

void WingedEdge::getOridinaryEdgesCoordinates(int i, double &x_1, double &x_2, double &y_1, double &y_2)
{
    x_1 = x[start_vertex[i]];
    x_2 = x[end_vertex[i]];
    y_1 = y[start_vertex[i]];
    y_2 = y[end_vertex[i]];

}

void WingedEdge::findPerpendicularBisector(double x_1, double y_1, double x_2, double y_2, double &m, double &b)
{
    if(fabs(y_1 - y_2)<1e-8){
        qDebug()<<"findPerpendicularBisector :divide by zero error! y_1 == y_2.";
        exit(-1);
    }
    m = (x_2-x_1)/(y_1-y_2);
    b = (x_1*x_1+y_1*y_1-x_2*x_2-y_2*y_2)/(2*(y_1-y_2));
}

vector<double> WingedEdge::get_g_x()
{
    return g_x;
}
vector<double> WingedEdge::get_g_y()
{
    return g_y;
}

vector<int> WingedEdge::get_w()
{
    return w;
}
vector<double> WingedEdge::get_x()
{
    return x;
}
vector<double> WingedEdge::get_y()
{
    return y;
}

vector<int> WingedEdge::get_right_polygon()
{
    return right_polygon;
}

vector<int> WingedEdge::get_left_polygon()
{
    return left_polygon;
}

vector<int> WingedEdge::get_start_vertex()
{
    return start_vertex;
}

vector<int> WingedEdge::get_end_vertex()
{
    return end_vertex;
}

vector<int> WingedEdge::getCw_predecessor() const
{
    return cw_predecessor;
}

void WingedEdge::setCw_predecessor(const vector<int> &value)
{
    cw_predecessor = value;
}

vector<int> WingedEdge::getCcw_predecessor() const
{
    return ccw_predecessor;
}

void WingedEdge::setCcw_predecessor(const vector<int> &value)
{
    ccw_predecessor = value;
}

vector<int> WingedEdge::getCw_successor() const
{
    return cw_successor;
}

void WingedEdge::setCw_successor(const vector<int> &value)
{
    cw_successor = value;
}

vector<int> WingedEdge::getCcw_successor() const
{
    return ccw_successor;
}

void WingedEdge::setCcw_successor(const vector<int> &value)
{
    ccw_successor = value;
}


bool compare_g_point(const g_point a, const g_point b)
{
    return a.x < b.x || (( fabs(a.x-b.x) < 1e-8 )&&(a.y < b.y));
}

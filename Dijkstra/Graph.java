 
import java.util.*;
 
 //Oliver Xia
 
public class Graph
{
        private int numVertices;
        // size of each block in pixels
        private final int YELLOW = 4;
 
 
 
        public Graph(int[][] adj)
        {
                numVertices = adj.length*adj[0].length;
        }
        //Node class
        private class Node{
                int x;
                int y;
                Node parent;
                private Node(int x, int y, Node parent){
                        this.x = x;
                        this.y = y;
                        this.parent = parent;
                }
        }
        //This method will create a 2D array for the purpose of keeping track of visited vertices
        public boolean[][] unVisited(int[][] x){
                boolean[][] temp = new boolean[x.length][x[0].length];
                for(int j = 0; j<x.length;j++){
                        for(int i = 0;i<x[0].length;i++){
                                temp[j][i] = false;
                        }
                }
                return temp;
        }
        //This method will find the neighbors of the given vertex
        public LinkedList<Node> allNeigh(int[][] solve, Node current){
                LinkedList<Node> temp = new LinkedList<Node>();
                if((current.x+1)<solve[0].length){
                        temp.add(new Node(current.x+1, current.y, current));
                       
                }
                if((current.y+1)<solve.length){
                        temp.add(new Node(current.x, current.y+1, current));
                     
                }
                if((current.y-1)>0){
                        temp.add(new Node(current.x, current.y-1, current));
                       
                }
                if((current.x-1)>0){
                        temp.add(new Node(current.x-1, current.y, current));
                        
                }
                return temp;
        }
        // This algorithm will find the shortest route to a destination
        public int[][] Dijkstra(int[][] solve, int startX, int startY, int endX, int endY){
                LinkedList<Node> traverse = new LinkedList<Node>();
                boolean[][] path = unVisited(solve);
                Node root = new Node(startX, startY, null);
                traverse.add(root);
 
                while(!traverse.isEmpty() && path[endY][endX]==false){

                        Node current = traverse.remove();
                        if(!(solve[current.y][current.x]==-1 || path[current.y][current.x]==true)){
                                path[current.y][current.x]=true;
                                traverse.addAll(allNeigh(solve, current));
 
                                if(current.x == endX && current.y==endY){
                                        while(current!=null){
                                                solve[current.y][current.x]=YELLOW;
                                                current = current.parent;
                                        }      
                                }
                        }
                }
 
                return solve;
        }
}
package Lab;
import java.io.*;
import java.util.*;
class Queue {
	int q[],f=0,r=0,size;
	Scanner sc=new Scanner(System.in);
	void insert(int n) {
		q=new int[10];
		for(int i=0;i<n;i++) {
			System.out.print("Enter "+i+" element");
			int ele=sc.nextInt();
			if(r+1>10) {
				System.out.println("Queue is full \n Lost Packet: "+ele);
				break;
			}
			else {
				r++;
				q[i]=ele;
			}
		}
	}
	void delete() {
		Thread t=new Thread();
		if(r==0)
			System.out.println("\nQueue Empty");
		else {
			for(int i=f;i<r;i++) {
				try {
					t.sleep(1000);
				}
				catch(Exception e) {}
				System.out.print("\nLeaked Packet:"+q[i]);
				f++;
			}
		}
		System.out.println();
		
	}
}
public class CongestionControl  extends Thread{
	
	public static void main(String[] args)  throws Exception{
		// TODO Auto-generated method stub
		Queue q=new Queue();
		Scanner src=new Scanner(System.in);
		System.out.println(" Enter the Packet to be sent: ");
		int size=src.nextInt();
		q.insert(size);
		q.delete();
	}

}

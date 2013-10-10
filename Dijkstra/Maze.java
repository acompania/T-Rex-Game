import java.awt.*;
import java.awt.event.*;
import java.applet.Applet;
import java.util.Iterator;
import javax.swing.*;
import java.util.Scanner;
import java.io.File;
//Oliver Xia
public class Maze extends Applet {

	// array that holds the maze info
	private int[][] maze;

	//number of rows and columns in the maze
	private int rows, cols;

	// initial size of maze - if bigger may go off window
	private final int MAXROWS = 20;
	private final int MAXCOLS = 30;
	private final int GREEN = 5;
	private final int RED = 3;
	private final int YELLOW = 4;
	private int startX;
	private int startY;
	private int endX;
	private int endY;
	private int click_counter;
	// size of each block in pixels
	private final int blkSize = 20;

	//inner class that displays the maze
	private MazeCanvas mazeField;

	// everything is put on this panel   
	private JPanel mazePanel;

	// label, textfield, string, and load button for the file
	private JLabel fileLabel;
	private JTextField fileText;
	String fileName;
	private JButton fileButton;
	private JButton GObutton;

	//this listener object responds to button events
	private ButtonActionListener buttonListener;
	private MouseEventListener mouseListener;

	// this method sets up the canvas and starts it off
	public void init() {
		System.out.println("Maze started"); // goes to console 

		mouseListener = new MouseEventListener();
		buttonListener = new ButtonActionListener();

		// the mazePanel is the panel that contains the maze interfaces, including
		// the buttons and output display
		mazePanel = new JPanel();
		// Y_AXIS layout places components from top to bottom, in order of adding
		mazePanel.setLayout(new BoxLayout(mazePanel, BoxLayout.Y_AXIS));

		// components for loading the filename
		fileLabel = new JLabel("File name:");
		mazePanel.add(fileLabel);
		fileText = new JTextField("", 20);
		mazePanel.add(fileText);
		fileButton = new JButton("Load File");
		mazePanel.add(fileButton);
		fileButton.addActionListener(buttonListener);
		GObutton = new JButton("GO");
		mazePanel.add(GObutton);
		GObutton.addActionListener(buttonListener);


		// this is where the maze is drawn
		// if you add more to this layout after the mazeField, 
		//   it may be below the bottom of the window, depending on window size
		mazeField = new MazeCanvas();
		mazePanel.add(mazeField);
		mazeField.addMouseListener(mouseListener);

		// now add the maze panel to the applet
		add(mazePanel);
	}

	// this object is triggered whenever a button is clicked
	private class ButtonActionListener implements ActionListener {
		public void actionPerformed(ActionEvent event) {

			// find out which button was clicked 
			Object source = event.getSource();

			if (source == fileButton)
			{
				fileName = fileText.getText();
				makeMaze(fileName);
			}
			if(source == GObutton){
				Graph solver = new Graph(maze);
				maze = solver.Dijkstra(maze, (startX/blkSize), (startY/blkSize), (endX/blkSize), (endY/blkSize));
				if(maze[endY/blkSize][endX/blkSize]==RED){
					JOptionPane.showMessageDialog(mazePanel, "Sorry cannot get there from where you started.");
				}
				maze[startY/blkSize][startX/blkSize] = GREEN;
				maze[endY/blkSize][endX/blkSize] = RED;
				mazeField.paint(mazeField.getGraphics());
			}
		} 
	}

	private class MouseEventListener implements MouseListener
	{
		public void mouseClicked(MouseEvent e)
		{
			// location on the mazeCanvas where mouse was clicked
			// upper-left is (0,0)
			if(click_counter == 0){
				startX = e.getX();
				startY = e.getY();
			}
			if(maze[startY/blkSize][startX/blkSize]!=-1){
				click_counter++;
				if(click_counter == 1){
					maze[startY/blkSize][startX/blkSize] = GREEN;
				}
				else if(click_counter == 2){
					endX = e.getX();
					endY = e.getY();
					maze[endY/blkSize][endX/blkSize] = RED;
				}
				else{}
			}
			mazeField.paint(mazeField.getGraphics());

		}

		public void mousePressed(MouseEvent e) { }
		public void mouseReleased(MouseEvent e) { }
		public void mouseEntered(MouseEvent e) { }
		public void mouseExited(MouseEvent e) { }
	}

	public boolean makeMaze(String fileName)
	{
		try
		{
			Scanner scanner = new Scanner(new File(fileName));
			rows = scanner.nextInt();
			cols = scanner.nextInt();
			maze = new int[rows][cols];
			click_counter=0;
			//fill out maze matrix
			for(int i=0; i<rows; i++)
			{
				for(int j=0; j<cols; j++)
				{
					maze[i][j] = scanner.nextInt();
				}
			}

			// now draw it
			mazeField.paint(mazeField.getGraphics());
			return true;
		}
		catch(Exception e)
		{
			return false;
		}
	}


	class MazeCanvas extends Canvas {
		// this class paints the output window 

		// the constructor sets it up
		MazeCanvas() {
			rows = MAXROWS;
			cols = MAXCOLS;
			maze = new int[MAXROWS][MAXCOLS];
			setSize(cols*blkSize, rows*blkSize);
			setBackground(Color.white);
		}

		public void paint(Graphics g)
		{
			g.setColor(Color.white);
			g.fillRect(0, 0, cols*blkSize, rows*blkSize);

			for (int i=0; i<rows; i++)
			{ 
				for (int j=0; j<cols; j++)
				{
					if (maze[i][j] == -1)
					{
						// location is a wall
						g.setColor(Color.black);
					}
					else if(maze[i][j] == GREEN)
					{
						g.setColor(Color.green);
					}
					else if(maze[i][j] == RED){
						g.setColor(Color.red);
					}
					else if(maze[i][j] == YELLOW){
						g.setColor(Color.yellow);
					}
					else{
						// location is clear
						g.setColor(Color.white);
					}
					// draw the location
					g.fillRect(j*blkSize, i*blkSize, blkSize, blkSize);
				}
			}
		}
	}
}
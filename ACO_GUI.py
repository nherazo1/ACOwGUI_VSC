import customtkinter as ctk
import tkinter as tk
from tkinter import messagebox
import shutil
from tkinter.filedialog import askopenfilename
import os
import random
from build.module_name import *

#### FOR MAP ----------------------------------- ####
import plotly as py
import plotly.graph_objs as go
from plotly.offline import init_notebook_mode, plot, iplot
import folium
import pandas as pd
from IPython.display import display
import webbrowser
#### FOR MAP ----------------------------------- ####


class MyGUI:
    def __init__(self):

        ctk.set_appearance_mode("dark") # either use "System", "dark", or "light"
        ctk.set_default_color_theme("dark-blue")
        self.data_loaded = False
        self.root = ctk.CTk()
        self.root.geometry("1000x600")
        self.root.title("Transportation Management Routing Tool")

        self.menubar = tk.Menu(self.root)

        self.filemenu = tk.Menu(self.menubar, tearoff=0)
        self.filemenu.add_command(label="Close", command=self.on_closing)
        self.filemenu.add_separator()
        self.filemenu.add_command(label="Close Without Question", command=exit)

        self.actionmenu = tk.Menu(self.menubar, tearoff=0)
        self.actionmenu.add_command(label="Show Message", command=self.show_message)

        self.menubar.add_cascade(menu=self.filemenu, label="File")
        self.menubar.add_cascade(menu=self.actionmenu, label="Action")

        self.root.config(menu=self.menubar)

        self.label = ctk.CTkLabel(self.root, text="Optimize your route planning process\n"
                                                  "Upload your deliveries data to obtain"
                                                  " the optimal routing plan!", font=('Arial', 18))
        self.label.pack(padx=10, pady=10)

        self.button = ctk.CTkButton(self.root, text="Load your data", font=('Arial', 18), command=self.load_data)
        self.button.pack(padx=10, pady=10)

        self.label = ctk.CTkLabel(self.root, text="Click the RUN button to optimize your route plan, "
                                                  "results will appear in the text box below", font=('Arial', 18))
        self.label.pack(padx=10, pady=10)

        self.sl_area = tk.Frame(self.root, width=90, height=185, bg='lightgrey')
        self.sl_area.pack(fill='both', padx=10, pady=10, expand=True)

        self.button = ctk.CTkButton(self.sl_area, text="RUN", font=('Arial', 18), command=self.run_router)

        self.textbox = tk.Text(self.sl_area, height=15, font=('Arial', 16))
        self.textbox.bind("<KeyPress>", self.shortcut)
        self.button.grid(padx=10, pady=10, column=0, row=0)
        self.textbox.grid(padx=10, pady=10, column=1, row=0)

        self.check_state = ctk.IntVar()

        self.check = ctk.CTkCheckBox(self.root, text="Show Messagebox", font=('Arial', 16), variable=self.check_state)
        self.check.pack(padx=10, pady=10)

        self.clearbtn = ctk.CTkButton(self.root, text="Clear route plan", font=('Arial', 18), command=self.clear)
        self.clearbtn.pack(padx=10, pady=10)

        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.root.mainloop()
        self.results = ""
        self.routes = []
        self.t_routes = []

    def show_message(self):
        if self.check_state.get() == 0:
            print(self.textbox.get('1.0', ctk.END))
        else:
            messagebox.showinfo(title="Message", message=self.textbox.get('1.0', ctk.END))

    def shortcut(self, event):
        if event.state == 4 and event.keysym == "Return": # Print the event to find out the values of different events
            self.show_message()

    def on_closing(self):
        if messagebox.askyesno(title="Die App schliessen?", message="Möchtet Ihr wirklich die App verlassen?"):
            self.root.destroy()

    def clear(self):
        self.textbox.delete('1.0', ctk.END)

    def load_data(self):
        src_file = askopenfilename(title='Select your file')
        dst_file = r"/Users/nherazo/Developer/PythonProjects/PyACOwGUI_VSC/data.csv"
        if src_file == '':
            messagebox.showinfo(title="Data loading failed", message="You did not select the file")
        else:
            shutil.copy(src_file, dst_file)
            messagebox.showinfo(title="Data successfully loaded", message="Your data have been successfully loaded "
                                                                          "for the route planning optimization process,"
                                                                          "you can proceed now to run the algorithm")
            self.data_loaded = True

    def print_results(self, mytext):
        self.textbox.delete("0.0", "end")
        self.textbox.insert(tk.INSERT, mytext)

    # Function to generate a random color in hex format  
    def random_color(self):  
        return "#{:06x}".format(random.randint(0, 0xFFFFFF)) 

    def displayMap(self):
        ### Load data ----------###
        data = 'data.csv'
        df = pd.read_csv(data)

        ### Create map ----------###
        routesMap = folium.Map(location=[df['xcoord.'].iloc[0], df['ycoord.'].iloc[0]], control_scale=True, zoom_start=15)
        fg = folium.FeatureGroup(name='routing')

        for i in range(len(df.index)):
            ndemand = df['demand'].iloc[i]
            f_ndemand = "{:,.0f}".format(ndemand)
            fg.add_child(folium.Marker(location=[df['xcoord.'].iloc[i], df['ycoord.'].iloc[i]], 
                                    popup=folium.Popup(f"""<h2>{df['cust no.'].iloc[i]}</h2> <br>
                                    <h2>Demand = {str(f_ndemand)}</h2> <br>
                                    <h2>ReadyTime = {df['ready time'].iloc[i]}</h2>""",
                                                        max_width=300)))
        routesMap.add_child(fg)

        # Define marker locations and labels  
        marker_locations = []
        for i in range(len(df.index)):
            marker_locations.append((df['xcoord.'].iloc[i], df['ycoord.'].iloc[i])) 

        
        color = [self.random_color() for i in self.t_routes]

        """sequence = []
        # Define the order in which to connect the markers (0-based index)  
        for i in range(len(self.t_routes)):
            for j in range(len(self.t_routes[i])):
                if self.t_routes[i][1] != 0:
                    sequence[i][j] = self.t_routes[i][j]  # This means we'll connect Marker 1 -> Marker 2 -> Marker 3 -> Marker 4 -> Marker 5  

        sequence = [[element for element in row] for row in self.t_routes]"""

        # Create a list of coordinates to pass to PolyLine  
        line_coordinates = []

        line_coordinates = [[marker_locations[i] for i in row] for row in self.t_routes]  

        # Add a PolyLine to connect the markers  
        for item in range(len(line_coordinates)):
            folium.PolyLine(locations=line_coordinates[item], color=color[item], weight=5, opacity=0.7).add_to(routesMap)  

        """# Optionally, add markers for the lines' start and end if needed  
        folium.Marker(  
            location=line_coordinates[0],   
            popup='Start',  
            icon=folium.Icon(color='green')  
        ).add_to(routesMap)  

        folium.Marker(  
            location=line_coordinates[-1],   
            popup='End',  
            icon=folium.Icon(color='red')  
        ).add_to(routesMap)"""

        routesMap.save('routesMap.html')

        # Get the absolute path to the HTML file  
        map_path = os.path.abspath('routesMap.html')  

        # Specify the Safari browser  
        safari_path = 'open -a "/Applications/Safari.app" %s'  # This works on MacOS  
        webbrowser.get(safari_path).open('file://' + map_path)

    def run_router(self):
        if self.data_loaded:
            #os.system("cmake .. && make && python ../test.py")
            os.system("/opt/homebrew/bin/cmake -B/Users/nherazo/Developer/PythonProjects/PyACOwGUI/build -S/Users/"
                      "nherazo/Developer/PythonProjects/PyACOwGUI && cd /Users/nherazo/Developer/PythonProjects/"
                      "PyACOwGUI/build && make")
            results_txt = "Your results are being processed, please wait, the algorithm will take a maximum time of 20 minutes. Your results will appear below..."
            self.print_results(results_txt)
            self.results = ACOfunct()
            results_txt = self.results
            self.routes = GetRoutes()

            #Trim routes that come with extra ceros from C++
            self.t_routes = [[element for j, element in enumerate(row) if j == 0 or element != 0] for row in self.routes]

            # Add DC node to the end of each route  
            value_to_add = 0  

            # Updating the list with the DC node added at the end of each route  
            self.t_routes = [row + [value_to_add] for row in self.t_routes]  


            self.print_results(results_txt)
            self.displayMap()
            print(self.t_routes)
        else:
            messagebox.showinfo(title="Data not available", message="You did not upload any data file")


m = MyGUI()

m



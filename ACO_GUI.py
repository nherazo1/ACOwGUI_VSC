import customtkinter as ctk
import tkinter as tk
from tkinter import messagebox
import shutil
from tkinter.filedialog import askopenfilename
import os
import ast
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

        self.runbtn = ctk.CTkButton(self.sl_area, text="RUN", font=('Arial', 18), command=self.run_router)
        self.vizbtn = ctk.CTkButton(self.sl_area, text="Visualize Route", font=('Arial', 18), command=self.displayMap)
        self.savebtn = ctk.CTkButton(self.sl_area, text="Save Route Planning", font=('Arial', 18), command=self.save_to_file)
        self.loadbtn = ctk.CTkButton(self.sl_area, text="Load last Route Planning", font=('Arial', 18), command=self.load_from_file)

        self.textbox = tk.Text(self.sl_area, height=15, font=('Arial', 16))
        self.textbox.bind("<KeyPress>", self.shortcut)
        self.runbtn.grid(padx=10, pady=5, column=0, row=0)
        self.vizbtn.grid(padx=10, pady=5, column=0, row=1)
        self.savebtn.grid(padx=10, pady=5, column=0, row=2)
        self.loadbtn.grid(padx=10, pady=5, column=0, row=3)
        self.textbox.grid(padx=10, pady=10, column=1, row=0, rowspan=4)

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
        self.routePlan = [] 

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
        dst_file = r"/home/nilson/Dokumente/Developer/PythonProjects/ACOwGUI_VSC/data.csv"
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
        
        if not hasattr(self, 't_routes'):
            messagebox.showinfo(title="Map display failed", message="There are no results to display")
            return 0
        
        ### Load data ----------###
        data = 'data.csv'
        df = pd.read_csv(data)

        RouteNo = launch_app()

        if not (isinstance(RouteNo, int) and 1 <= RouteNo <= len(self.t_routes)):
            messagebox.showinfo(title="Route visualization failed", message="You did not select a valid route")
            return 0

        sel_df = df[df["cust no."].isin(self.t_routes[RouteNo - 1])]

        ### Create map ----------###
        routesMap = folium.Map(location=[sel_df['xcoord.'].iloc[0], sel_df['ycoord.'].iloc[0]], control_scale=True, zoom_start=15)
        fg = folium.FeatureGroup(name='routing')

        for i in range(len(sel_df.index)):
            ndemand = sel_df['demand'].iloc[i]
            f_ndemand = "{:,.0f}".format(ndemand)
            fg.add_child(folium.Marker(location=[sel_df['xcoord.'].iloc[i], sel_df['ycoord.'].iloc[i]], 
                                    popup=folium.Popup(f"""<h2>{sel_df['cust no.'].iloc[i]}</h2> <br>
                                    <h2>Demand = {str(f_ndemand)}</h2> <br>
                                    <h2>ReadyTime = {sel_df['ready time'].iloc[i]}</h2>""",
                                                        max_width=300)))
        routesMap.add_child(fg)

        # Define marker locations and labels  
        marker_locations = []
        for i in range(len(df.index)):
            marker_locations.append((df['xcoord.'].iloc[i], df['ycoord.'].iloc[i])) 

        color = self.random_color()

        # Create a list of coordinates to pass to PolyLine  
        line_coordinates = [marker_locations[i] for i in self.t_routes[RouteNo - 1]]

        # Add a PolyLine to connect the markers  
        folium.PolyLine(locations=line_coordinates, color=color, weight=5, opacity=0.7).add_to(routesMap)  

        # Optionally, add markers for the lines' start and end if needed  
        folium.Marker(  
            location=line_coordinates[0],   
            popup='Start',  
            icon=folium.Icon(color='green')  
        ).add_to(routesMap)  

        folium.Marker(  
            location=line_coordinates[-1],   
            popup='End',  
            icon=folium.Icon(color='red')  
        ).add_to(routesMap)

        routesMap.save('routesMap.html')

        # Get the absolute path to the HTML file  
        map_path = os.path.abspath('routesMap.html')  

        # Specify the Safari browser  
        browser_path = 'xdg-open'  # This works on Linux  
        webbrowser.get(browser_path).open('file://' + map_path)

    def save_to_file(self):
        fileName = "routePlan.txt"
        if not hasattr(self, 'routePlan'):
            messagebox.showinfo(title="Save results to file failed", message="There are no results to save!")
        else:
            try:
                save_route_plan_to_file(self.routePlan, fileName)
                messagebox.showinfo(title="Results successfully saved", message="Your results were stored in the system")
            except ValueError:
                messagebox.showinfo(title="Save results to file failed", message="There are no results to save!")

    def load_from_file(self):
        fileName = "routePlan.txt"
        try:
            description, routes = load_routes_from_file(fileName)
            self.t_routes = routes
            self.print_results(description)
        except ValueError:
            messagebox.showinfo(title="Load results from file failed", message="There are no results to load!")

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

            #Save results
            self.routePlan = RoutePlan(self.results, self.t_routes)
        else:
            messagebox.showinfo(title="Data not available", message="You did not upload any data file")

class RouteSelectApp(ctk.CTk):  
    def __init__(self):  
        super().__init__()  
        self.minsize(400, 300)
        self.value = None  # Store the integer value here  
        self.frame = ctk.CTkFrame(self)
        # Add some content to the frame  
        self.title("Route Visualization Selector")  
        label = ctk.CTkLabel(self.frame, text="Select the route to visualize")  
        label.pack(pady=20) 
        self.frame.pack(padx=20, pady=20)  
        self.entry = ctk.CTkEntry(self.frame, placeholder_text="Enter a No. of a route (integer)", text_color='grey', width=300, height=40)  
        self.entry.pack(pady=10)  
        self.submit_button = ctk.CTkButton(self.frame, text="Submit route", command=self.get_integer)  
        self.submit_button.pack(pady=10)  

    def get_integer(self):  
        try:  
            self.value = int(self.entry.get())
            self.after(100, self.destroy())  # Schedule closing the application with a slight delay after submission
            self.quit()
        except ValueError:  
            messagebox.showinfo(title="Value error", message="Please enter a valid integer.") 

    def run_app(self):  
        self.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.mainloop()  # Start the application loop
        return self.value  # Return the integer value after the app closes  
    
    def on_closing(self):
        if messagebox.askyesno(title="Die App schliessen?", message="Möchtet Ihr wirklich die App verlassen?"):
            self.destroy()

def launch_app():  
    app = RouteSelectApp()  
    result = app.run_app()  # This will run the app and wait for user input  
    return result  # Return the result to C++

class RoutePlan:  
    def __init__(self, description, route):  
        self.description = description  
        self.route = route  

def save_route_plan_to_file(routePlan, file_name):  
    with open(file_name, 'w') as ostream:    
        description = routePlan.description.replace(' ', '_')  
        ostream.write(f"{description} {routePlan.route}\n")

def load_routes_from_file(file_name):  
    if not os.path.exists(file_name):  
        return []  
    
    description = []
    with open(file_name, 'r') as istream:    
        for line in istream:  
            line = line.strip()
            if " " in line:
                routes = line.rsplit(' ', 1)
                results = "\n".join(description)
                routes = "".join(routes)
                routes_l = ast.literal_eval(routes)
                return results, routes_l
            line = line.replace('_', ' ') 
            description.append(line) 
        #routePlan.append(RoutePlan(description, routes))  
    
    return results, None


m = MyGUI()

m

import customtkinter as ctk
import tkinter as tk
from tkinter import messagebox
import shutil
from tkinter.filedialog import askopenfilename
import os
from build.module_name import *


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
        self.results = "These are your results: ..."

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
        self.textbox.insert(tk.INSERT, mytext)

    def run_router(self):
        if self.data_loaded:
            #os.system("cmake .. && make && python ../test.py")
            os.system("/opt/homebrew/bin/cmake -B/Users/nherazo/Developer/PythonProjects/PyACOwGUI/build -S/Users/"
                      "nherazo/Developer/PythonProjects/PyACOwGUI && cd /Users/nherazo/Developer/PythonProjects/"
                      "PyACOwGUI/build && make")
            self.results = ACOfunct()
            self.print_results(self.results)
        else:
            messagebox.showinfo(title="Data not available", message="You did not upload any data file")


m = MyGUI()

m






import customtkinter
from tkinter import messagebox

class App(customtkinter.CTk):  
    def __init__(self):  
        super().__init__()  
        self.minsize(400, 300)
        self.value = None  # Store the integer value here  
        self.frame = customtkinter.CTkFrame(self)
        # Add some content to the frame  
        self.title("Fleet Size Setup")  
        label = customtkinter.CTkLabel(self.frame, text="Configure Vehicle Fleet Size")  
        label.pack(pady=20) 
        self.frame.pack(padx=20, pady=20)  
        self.entry = customtkinter.CTkEntry(self.frame, placeholder_text="Enter a No. of vehicles (integer)", text_color='grey', width=300, height=40)  
        self.entry.pack(pady=10)  
        self.entry.focus_set()
        self.submit_button = customtkinter.CTkButton(self.frame, text="Submit fleet Size", command=self.get_integer)
        self.submit_button.pack(pady=10)  
        self.bind('<Return>', self.get_integer)
        self.bind('<KP_Enter>', self.get_integer)
        self.protocol("WM_DELETE_WINDOW", self.onClose)

    def get_integer(self, event=None):  
        try:  
            self.value = int(self.entry.get())
            self.after(100, self.destroy())  # Schedule closing the application with a slight delay after submission
            self.quit()
        except ValueError:  
            messagebox.showinfo(title="Value error", message="Please enter a valid integer.") 

    def run_app(self):  
        self.mainloop()  # Start the application loop
        return self.value  # Return the integer value after the app closes  
    
    def onClose(self, event=None):
        self.value = -1
        self.after(100, self.destroy())  # Schedule closing the application with a slight delay after submission
        self.quit()  


def launch_app():  
    app = App()  
    result = app.run_app()  # This will run the app and wait for user input  
    return result  # Return the result to C++

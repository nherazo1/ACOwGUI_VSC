import pandas as pd
import folium
from IPython.display import display
import webbrowser
import os

def displayMap():
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

        # Define the order in which to connect the markers (0-based index)  
        sequence = [0 , 79 , 75 , 86 , 77 , 83 , 88 , 87 , 81 , 84 , 82 , 
                    85 , 57 , 29 , 55 , 52 , 49 , 51 , 43 , 47 , 42 , 
                    44 , 61 , 58 , 60 , 1 , 59 , 56 , 8 , 10 , 7 , 
                    15 , 13 , 12 , 23 , 21 , 20 , 14 , 9 , 18 , 19 , 
                    16 , 17 , 6 , 78 , 0 ]  # This means we'll connect Marker 1 -> Marker 2 -> Marker 3 -> Marker 4 -> Marker 5  

        # Create a list of coordinates to pass to PolyLine  
        line_coordinates = [marker_locations[i] for i in sequence]  

        # Add a PolyLine to connect the markers  
        folium.PolyLine(locations=line_coordinates, color='blue', weight=5, opacity=0.7).add_to(routesMap)  

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
        safari_path = 'open -a "/Applications/Safari.app" %s'  # This works on MacOS  
        webbrowser.get(safari_path).open('file://' + map_path)

        #webbrowser.open('routesMap.html')

displayMap()

# Test opening a known URL  
#webbrowser.open('http://www.google.com')  

from flask import Flask, render_template, request, jsonify
import pandas as pd
import plotly.express as px
from datetime import datetime

app = Flask(__name__)

# Store data in a DataFrame
df = pd.DataFrame(columns=["timestamp", "ldr", "temperature", "humidity"])

@app.route('/')
def home():
    # Visualization using Plotly
    fig = px.line(df, x="timestamp", y="ldr", title="LDR Sensor Value Over Time")
    fig.update_layout(xaxis_title="Time", yaxis_title="LDR Value")
    graph_html = fig.to_html(full_html=False)

    # Visualize temperature and humidity
    fig2 = px.line(df, x="timestamp", y=["temperature", "humidity"], title="Temperature and Humidity Over Time")
    fig2.update_layout(xaxis_title="Time", yaxis_title="Value")
    graph_html2 = fig2.to_html(full_html=False)

    return render_template('index.html', graph_html=graph_html, graph_html2=graph_html2)

@app.route('/sensor_data', methods=['POST'])
def sensor_data():
    global df
    # Get data from ESP32 POST request
    ldr = request.form['ldr']
    temperature = request.form['temperature']
    humidity = request.form['humidity']

    # Add data to DataFrame with current timestamp
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    new_data = {"timestamp": timestamp, "ldr": ldr, "temperature": temperature, "humidity": humidity}
    df = df.append(new_data, ignore_index=True)

    return jsonify({"message": "Data received successfully"}), 200

if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0', port=5000)

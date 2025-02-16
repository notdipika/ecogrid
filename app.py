from flask import Flask, request, jsonify, render_template
from datetime import datetime
import sqlite3

app = Flask(__name__)

# Database setup
DB_NAME = "sensor_data.db"

def init_db():
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS sensor_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ldr INTEGER,
            temperature REAL,
            humidity REAL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    """)
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS alerts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            message TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    """)
    conn.commit()
    conn.close()

# Variable to store the latest command
latest_command = ""

@app.route("/send_command", methods=["POST"])
def send_command():
    global latest_command
    command = request.form.get("command")
    if command:
        latest_command = command
        return "Command received", 200
    return "No command provided", 400

@app.route("/get_command", methods=["GET"])
def get_command():
    global latest_command
    return latest_command, 200

@app.route("/sensor_data", methods=["POST"])
def sensor_data():
    ldr = request.form.get("ldr")
    temperature = request.form.get("temperature")
    humidity = request.form.get("humidity")

    if not all([ldr, temperature, humidity]):
        return "Missing sensor data", 400

    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute("INSERT INTO sensor_data (ldr, temperature, humidity) VALUES (?, ?, ?)", (ldr, temperature, humidity))
    conn.commit()
    conn.close()

    return "Sensor data received", 200

@app.route("/alert", methods=["POST"])
def alert():
    alert_message = request.form.get("alert")

    if not alert_message:
        return "No alert message provided", 400

    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute("INSERT INTO alerts (message) VALUES (?)", (alert_message,))
    conn.commit()
    conn.close()

    return "Alert received", 200

@app.route("/")
def index():
    # Fetch latest sensor data
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute("SELECT ldr, temperature, humidity, timestamp FROM sensor_data ORDER BY timestamp DESC LIMIT 1")
    latest_data = cursor.fetchone()

    # Fetch latest alerts
    cursor.execute("SELECT message, timestamp FROM alerts ORDER BY timestamp DESC LIMIT 5")
    latest_alerts = cursor.fetchall()
    conn.close()

    return render_template("index.html", data=latest_data, alerts=latest_alerts)

if __name__ == "__main__":
    init_db()
    app.run(host="0.0.0.0", port=5000, debug=True)

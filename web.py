from flask import Flask, render_template, request, jsonify,session,redirect, url_for
from mysql.connector import pooling, Error
from functools import wraps 
import requests


# Setup MySQL connection pool
pool = pooling.MySQLConnectionPool(
    pool_name="mypool",
    pool_size=5,
    pool_reset_session=True,
    host="127.0.0.1",
    user="root",
    password="0786901729",
    database="Group_12"
)

app = Flask(__name__)
app.secret_key = 'ENGINEERINGPROGRAMMING'

print("Connection pool created successfully.")

#------------------------------------------------Decorator Function----------------------------------------------
def login_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if not session.get('logged_in'):
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function

#------------------------------------------------Protect Routes---------------------------------------------
# Home page route
@app.route('/', methods=["GET"])
def home():
    return render_template("home.html")


@app.route('/login', methods=["POST", "GET"])
def login():
    if request.method == "POST":
        username = request.form["username"]
        password = request.form["password"]
        connection = pool.get_connection()
        cursor = connection.cursor()
        cursor.execute("SELECT * FROM Registration WHERE User_Password = %s AND Username = %s", (password, username,))
        result = cursor.fetchone()
        cursor.close()
        connection.close()
        if result:
            session['logged_in'] = True
            session['username'] = username
            return render_template("verify_detail.html", username=username, exists=True)
        else:
            return render_template("verify_detail.html", exists=False)
    return render_template("Login.html")

@app.route('/register', methods=["POST", "GET"])
def register():
    if request.method == "POST":
        username = request.form["username"]
        password = request.form["password"]
        email = request.form["email"]
        connection = pool.get_connection()
        cursor = connection.cursor()
        cursor.execute("INSERT INTO Registration(Username, User_Password, Student_Email) VALUES(%s, %s, %s)", (username, password, email))
        connection.commit()
        cursor.close()
        connection.close()
        return render_template("result.html", username=username, email=email)
    return render_template("register.html")

#----------------------------------------------HELPER FUNTION---------------------------------#
def fetch_latest_data(table_name, data_column):
    try:
        connection = pool.get_connection()
        cursor = connection.cursor(dictionary=True)
        query = f"SELECT {data_column}, lastUpdated FROM {table_name} ORDER BY lastUpdated DESC LIMIT 1"
        cursor.execute(query)
        row = cursor.fetchone()
        cursor.close()
        connection.close()

        if row:
            return jsonify({
                "timestamp": row["lastUpdated"].strftime('%Y-%m-%d %H:%M:%S'),
                "value": row[data_column]
            })
        else:
            return jsonify({"error": "No data found"}), 404
    except Error as err:
        print(f"Database error: {err}")
        return jsonify({"error": str(err)}), 500
    
#-------------------------------------------------API'S-----------------------------------------------#

@app.route('/humidity_data')
def humidity_data():
    return fetch_latest_data("humidity", "relative_humidity")

@app.route('/temp_data')
def temp_data():
    return fetch_latest_data("temperature", "temp_value")

@app.route('/ultrasonic_data')
def ultrasonic_data():
    return fetch_latest_data("ultrasonic", "altra_value")

@app.route('/pir_data')
def pir_data():
    return fetch_latest_data("pir", "PIR_STATUS")

@app.route('/ldr1_data')
def ldr1_data():
    return fetch_latest_data("ldr_1", "Ldr_1_data")


#-------------------------------------------Render Templates Routes---------------------------------------#
@app.route('/temperature', methods=["POST", "GET"])
@login_required
def temperature_page():
    return render_template("temperature.html")

@app.route('/humidity', methods=["POST", "GET"])
@login_required
def humidity_page():
    return render_template("humidity.html")

@app.route('/temperature_humidity', methods=["POST", "GET"])
@login_required
def temperature_humid_page():
    return render_template("temp_hum.html")

@app.route('/pir', methods=["POST", "GET"])
@login_required
def pir():
    return render_template("pir.html")

@app.route('/ultra', methods=["POST", "GET"])
@login_required
def ultr():
    return render_template("ultrasonic.html")

@app.route('/ldr_1', methods=["POST", "GET"])
@login_required
def Ldr_1():
    return render_template("ldr_1.html")

@app.route('/admin', methods=["POST", "GET"])
@login_required
def admin():
    connection = pool.get_connection()
    cursor = connection.cursor()
    cursor.execute("SELECT * FROM Registration")
    data = cursor.fetchall()
    cursor.close()
    connection.close()
    return render_template("admin.html", data=data, stream_url=ESP32_STREAM_URL)

# -------------------- ESP32 Sensor Data Route --------------------
@app.route('/sensor-data', methods=['POST'])
def esp32_data():
    try:
        data = request.get_json()
        id = data['id']
        Humidity = data['humidity']
        Temperature = data['temperature']

        connection = pool.get_connection()
        cursor = connection.cursor()

        cursor.execute("INSERT INTO humidity(relative_humidity) VALUES (%s)", (Humidity,))
        cursor.execute("INSERT INTO temperature(temp_value) VALUES (%s)", (Temperature,))
        connection.commit()
        cursor.close()
        connection.close()

        print("Received data:", data)
        return jsonify({"status": "success"}), 200

    except Exception as e:
        print(f"Error inserting sensor data: {e}")
        return jsonify({"status": "failure", "error": str(e)}), 500

# -------------------- ESP32 Sensor Data Route --------------------
@app.route('/pir-data', methods=['POST'])
def esp32_data_pir():
    try:
        data_pir = request.get_json()
        id = data_pir['id']
        PIR_info = data_pir['motion_detected']

        connection = pool.get_connection()
        cursor = connection.cursor()
        cursor.execute("INSERT INTO pir(PIR_STATUS) VALUES (%s)", (PIR_info,))
        connection.commit()
        cursor.close()
        connection.close()

        print("Received data:", data_pir)
        return jsonify({"status": "success"}), 200

    except Exception as e:
        print(f"Error inserting sensor data: {e}")
        return jsonify({"status": "failure", "error": str(e)}), 500
    
@app.route('/ultr-data', methods=['POST'])
def esp32_data_ultrasonic():
    try:
        data_ultra = request.get_json()
        sensor_id = data_ultra.get('id')
        ultr_distance = data_ultra.get('distance')

        connection = pool.get_connection()
        cursor = connection.cursor()
        cursor.execute("INSERT INTO ultrasonic(altra_value) VALUES (%s)", (ultr_distance,))
        connection.commit()
        cursor.close()
        connection.close()

        print("Received ultrasonic data:", data_ultra)
        return jsonify({"status": "success"}), 200

    except Exception as e:
        print(f"Error inserting ultrasonic data: {e}")
        return jsonify({"status": "failure", "error": str(e)}), 500

    
@app.route('/ldr-data', methods=['POST'])
def esp32_data_ldr():
    try:
        data_ldr = request.get_json()
        id = data_ldr['id']
        ldr_1_value = data_ldr['ldr_1_value']

        connection = pool.get_connection()
        cursor = connection.cursor()
        cursor.execute("INSERT INTO Ldr_1(Ldr_1_data) VALUES (%s)", (ldr_1_value,))
        connection.commit()
        cursor.close()
        connection.close()

        print("Received data:", data_ldr)
        return jsonify({"status": "success"}), 200

    except Exception as e:
        print(f"Error inserting sensor data: {e}")
        return jsonify({"status": "failure", "error": str(e)}), 500



ESP32_STREAM_URL = "http://192.186.43.181/stream"  

latest_data = {}

@app.route('/cam')
def index():
    return render_template("cam.html", stream_url=ESP32_STREAM_URL)




@app.route('/data-camera', methods=['POST'])
def receive_data():
    global latest_data
    latest_data = request.get_json()
    print("Received:", latest_data)
    return jsonify({"status": "ok"})

@app.route('/get-data-camera')
def get_data():
    return jsonify(latest_data)


    
#--------------------------------------------Logout Route-------------------------------------------#
@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('login'))
    
#--------------------------------------------Message Route------------------------------------------#
if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0', port=5000)

    
    
    
 
 
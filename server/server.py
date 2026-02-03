from flask import Flask, jsonify
from datetime import datetime
import pytz
import requests
import os
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__)

@app.route('/time', methods=['GET'])
def get_time():
    # Get current time in different formats
    now = datetime.now()
    
    return jsonify({
        'datetime': now.strftime('%Y-%m-%d %H:%M:%S'),
        'weekday': now.weekday()+1, # Plus 1 to account array indexing in STM32
    })

@app.route('/time/utc', methods=['GET'])
def get_utc_time():
    # Get UTC time
    utc_now = datetime.now(pytz.UTC)
    
    return jsonify({
        'datetime': utc_now.strftime('%Y-%m-%d %H:%M:%S'),
        'timestamp': int(utc_now.timestamp()),
        'timezone': 'UTC'
    })
    
@app.route('/weather/<zip>', methods=['GET'])
def get_weather(zip):
    r = requests.get(f'http://api.weatherapi.com/v1/current.json?key={os.getenv("WEATHER_API_KEY")}&q={zip}')
    res = {
        "humidity": r.json()['current']['humidity'],
        "temp_f": r.json()['current']['temp_f'],
        "precip_in": r.json()['current']['precip_in'],
    }
    return jsonify(res)

if __name__ == '__main__':
    # Run on all interfaces so ESP can access it
    app.run(host='0.0.0.0', port=5000, debug=True)

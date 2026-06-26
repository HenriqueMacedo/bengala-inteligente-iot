from flask import Flask, request, jsonify, render_template_string
import csv
import os
from datetime import datetime

app = Flask(__name__)
CSV_FILE = "dados_bengala.csv"

if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp", "distancia", "ax", "ay", "az", "evento"])

@app.route("/dados", methods=["POST"])
def receber_dados():
    dados = request.json
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(CSV_FILE, "a", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            timestamp,
            dados.get("distancia", 0),
            dados.get("ax", 0),
            dados.get("ay", 0),
            dados.get("az", 0),
            dados.get("evento", "")
        ])
    return jsonify({"status": "ok"})

@app.route("/")
def dashboard():
    dados = []
    with open(CSV_FILE, "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            dados.append(row)
    dados = dados[-50:]  # últimos 50 registros

    html = """
    <!DOCTYPE html>
    <html>
    <head>
        <title>Bengala Inteligente</title>
        <meta http-equiv="refresh" content="3">
        <style>
            body { font-family: Arial, sans-serif; padding: 20px; background: #f4f4f4; }
            h1 { color: #333; }
            table { width: 100%; border-collapse: collapse; background: white; }
            th { background: #333; color: white; padding: 10px; }
            td { padding: 8px; border-bottom: 1px solid #ddd; text-align: center; }
            tr:hover { background: #f0f0f0; }
            .alerta { color: red; font-weight: bold; }
        </style>
    </head>
    <body>
        <h1>Dashboard - Bengala Inteligente</h1>
        <p>Atualizando a cada 3 segundos...</p>
        <table>
            <tr>
                <th>Timestamp</th>
                <th>Distancia (cm)</th>
                <th>Ax</th>
                <th>Ay</th>
                <th>Az</th>
                <th>Evento</th>
            </tr>
            {% for row in dados|reverse %}
            <tr>
                <td>{{ row.timestamp }}</td>
                <td>{{ row.distancia }}</td>
                <td>{{ row.ax }}</td>
                <td>{{ row.ay }}</td>
                <td>{{ row.az }}</td>
                <td class="{{ 'alerta' if row.evento else '' }}">{{ row.evento if row.evento else '-' }}</td>
            </tr>
            {% endfor %}
        </table>
    </body>
    </html>
    """
    return render_template_string(html, dados=dados)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False)
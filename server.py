from flask import Flask, send_file

app = Flask(__name__)

@app.route("/firmware")
def firmware():
    return send_file("firmware.enc", as_attachment=True)

@app.route("/meta")
def meta():
    return send_file("meta.enc", as_attachment=True)

app.run(host="0.0.0.0", port=6000)

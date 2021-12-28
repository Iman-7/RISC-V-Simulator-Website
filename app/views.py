from app import app
import subprocess
from flask import render_template, request, redirect
from datetime import datetime
import os
from flask import Flask, flash, request, redirect, url_for
from werkzeug.utils import secure_filename

@app.template_filter("clean_date")
def clean_date(dt):
    return dt.strftime("%d %b %Y")

@app.route("/")
def home():
    return render_template("public/index.html")

@app.route("/about")
def about():
    return render_template("public/about.html")


UPLOAD_FOLDER = 'D:\\C++ projects\\Assembly_pro\\app\\static\\uploaded'
ALLOWED_EXTENSIONS = {'txt'}

# app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route('/test', methods=['GET', 'POST'])
def upload_file():
    if request.method == 'POST':
        # check if the post request has the file part
        if 'file' not in request.files:
            flash('No file part')
            return redirect(request.url)
        file = request.files['file']
        # If the user does not select a file, the browser submits an
        # empty file without a filename.
        if file.filename == '':
            flash('No selected file')
            return redirect(request.url)
        if file and allowed_file(file.filename):
            file_name = os.path.join(app.config['UPLOAD_FOLDER'], secure_filename(file.filename))
            file.save(file_name)

            return redirect(url_for('output', name=file_name))
    return render_template("public/yayyy.html")


@app.route('/output/<name>')  
def output(name):
    command ='.\main "{}"'.format(name)
    print("hiiiiii", command)
    out = subprocess.check_output(command, shell = True).decode("utf-8")
    # decode s to a normal string
    print(out)
    return render_template("public/output.html", out=out)
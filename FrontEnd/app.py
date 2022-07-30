from wsgiref.util import request_uri
from flask import Flask, render_template, request
from core import System
import os

app = Flask(__name__)

os.system("rm data/*")
sys = System('./code')

init_cmd = "[1] add_user -u admin -p 123456 -g 10 -m shen-dong@sjtu.edu.cn -n 董珅"
sys.write(init_cmd)
sys.read()

sys.write("[3] login -u admin -p 123456")
sys.read()

init_cmd = "[2] add_user -c admin -u hnyls2002 -p asdasd -g 9 -m hnyls2002@gmail.com -n 尹良升"
sys.write(init_cmd)
sys.read()

sys.write("[4] login -u hnyls2002 -p asdasd")
sys.read()

sys.write("[5] add_train -s 北京市|天津市|山东省兖州市|湖北省荆州市|浙江省海宁市|吉林省松原市|安徽省宿州市 -d 06-17|08-31 -x 14:47 -m 66757 -p 153|326|121|375|337|438 -i ACM2021 -o 7|9|6|9|7 -t 49|31|291|295|258|95 -y L -n 7")
sys.read()

sys.write("[5] release_train -i ACM2021")
sys.read()

sys.write("[5] add_train -s 北京市|上海市|湖南省长沙市|浙江杭州市|江苏南京市|交大电院|交大理科群楼 -d 06-17|08-31 -x 14:47 -m 66757 -p 153|326|121|375|337|438 -i SJTU2021 -o 7|9|6|9|7 -t 49|31|291|295|258|95 -y L -n 7")
sys.read()

sys.write("[5] release_train -i SJTU2021")
sys.read()

sys.write("[5] add_train -s 上海虹桥|杭州东|南昌西|长沙南 -d 06-17|08-31 -x 8:00 -m 19260718 -p 0|0|0|0 -i G817 -o 15|26 -t 45|120|60 -y G -n 4")
sys.read()

sys.write("[5] release_train -i G817")
sys.read()

sys.write("[5] add_train -s D22|D19 -d 06-17|08-31 -x 8:00 -m 10260817 -p 0 -i Dorm -o | -t 120 -y D -n 2")
sys.read()

sys.write("[5] release_train -i Dorm")
sys.read()

sys.write("[5] add_train -s D22|D19 -d 06-17|08-31 -x 8:00 -m 10260817 -p 0 -i Dorm -o | -t 120 -y D -n 2")
sys.read()

sys.write("[5] release_train -i Dorm")
sys.read()

sys.write("[5] add_train -s freshman|unfreshman -d 06-17|09-31 -x 8:00 -m 10260817 -p 0 -i SJTU -o | -t 120 -y D -n 2")
sys.read()

sys.write("[5] release_train -i SJTU")
sys.read()

sys.write(
    "[6] buy_ticket -f 北京市 -i ACM2021 -u admin -n 5 -d 08-16 -q true -t 山东省兖州市")
sys.read()

sys.write(
    "[7] buy_ticket -n 5 -f 北京市 -t 安徽省宿州市 -d 08-08 -u hnyls2002 -i ACM2021 -q true")
sys.read()

sys.write(
    "[8] buy_ticket -t 湖北省荆州市 -f 山东省兖州市 -n 3 -d 07-13 -i ACM2021 -q true -u admin")
sys.read()

sys.write(
    "[9] buy_ticket -u hnyls2002 -t 吉林省松原市 -n 3 -i ACM2021 -d 06-23 -f 天津市")
sys.read()

sys.write(
    "[10] buy_ticket -i ACM2021 -t 安徽省宿州市 -u admin -q true -d 07-19 -n 1 -f 北京市")
sys.read()


cur_user = 'admin'


@app.route('/', methods=["GET", "POST"])
def index():
    return render_template("log_in.html", alert_flag="false")


@app.route('/login_action', methods=["GET", "POST"])
def login():
    form = request.form.to_dict()
    res = sys.login(form['username'], form['password'])
    global cur_user
    if res[0] == '0':
        cur_user = form['username']
        return render_template("query_ticket.html")
    else:
        cur_user = ''
        return render_template("log_in.html", alert_flag='true', res=res)


@app.route('/register', methods=["GET", "POST"])
def register():
    form = request.form.to_dict()
    res = sys.register(form['username'], form['realname'],
                       form['password'], form['emailaddr'])
    if res[0] == True:
        return render_template("log_in.html", info_flag='true')
    else:
        return render_template("register.html", alert_flag='true', res=res)


@app.route('/to_modify_profile', methods=["GET", "POST"])
def to_modify_profile():
    return render_template("modify_profile.html", target_user=cur_user)


@app.route('/root_modify_profile', methods=["GET", "POST"])
def root_modify_profile():
    form = request.form.to_dict()
    if cur_user != 'admin':
        return render_template("query_user.html", alert_flag='true', info_flag='false')
    else:
        return render_template("modify_profile.html", target_user=form['target_user'])


@app.route('/modify_profile', methods=["GET", "POST"])
def modify_profile():
    form = request.form.to_dict()
    res = sys.modify_profile(
        form['target_user'], form['new_realname'], form['new_email'], form['new_password'])
    if res[0] == True:
        return render_template("query_user.html", info_flag='true', res=res[1])
    elif res[0] == False:
        return render_template("query_user.html", info_flag='error', res=res[1])


@app.route('/to_query_user', methods=["GET", "POST"])
def to_query_user():
    return render_template("query_user.html", cur_user=cur_user)


@app.route('/query_user', methods=["GET", "POST"])
def query_user():
    form = request.form.to_dict()
    res = sys.query_user(form['Username'])
    if res[0] == True:
        return render_template("query_user.html", info_flag='true', res=res[1])
    elif res[0] == False:
        return render_template("query_user.html", info_flag='error', res=res[1])


@app.route('/to_register', methods=["GET", "POST"])
def to_register():
    return render_template("register.html")


@app.route('/to_login', methods=["GET", "POST"])
def to_login():
    return render_template("log_in.html")


@app.route('/show_self_profile', methods=["GET", "POST"])
def show_self_profile():
    res = sys.query_self_profile(cur_user)
    return render_template("profile.html", res=res)


@app.route('/log_out', methods=["GET", "POST"])
def log_out():
    return render_template("log_in.html")


@app.route('/to_run_manager', methods=["GET", "POST"])
def to_run_manager():
    return render_template("add_train.html")


@app.route('/to_query_order', methods=["GET", "POST"])
def to_query_order():
    res = sys.query_order(cur_user)
    print(res)
    return render_template("query_order.html", res=res)


@app.route('/to_refund_ticket', methods=["GET", "POST"])
def to_refund_ticket():
    res = sys.query_order(cur_user)
    print(res)
    return render_template("refund_ticket.html", alert_flag='show', res=res)


@app.route('/refund_ticket', methods=["GET", "POST"])
def refund_ticket():
    form = request.form.to_dict()
    res = sys.refund_ticket(cur_user, form['number'])
    print(res)
    if res[0] == True:
        return render_template("refund_ticket.html", alert_flag='show', res=res[1])
    else:
        return render_template("refund_ticket.html", alert_flag='true', res=res[1], alert_info=res[2])


@app.route('/to_buy_ticket', methods=["GET", "POST"])
def to_buy_ticket():
    return render_template("buy_ticket.html")


@app.route('/buy_ticket', methods=["GET", "POST"])
def buy_ticket():
    form = request.form.to_dict()
    res = sys.buy_ticket(cur_user, form['trainId'], form['number'],
                         form['datepicker'], form['from'], form['to'])
    return render_template("buy_ticket.html", alert_info=res[1])


@app.route('/to_query_train', methods=["GET", "POST"])
def to_query_train():
    return render_template("query_train.html")


@app.route('/show_train', methods=["GET", "POST"])
def query_train():
    form = request.form.to_dict()
    res = sys.query_train(form['datepicker'], form['trainId'])
    if res[0] == True:
        return render_template("show_train.html", res=res[1])
    else:
        return render_template("query_train.html", alert_flag='true', alert_info=res[1])


@app.route('/to_query_ticket', methods=["GET", "POST"])
def to_query_ticket():
    return render_template("query_ticket.html")


@app.route('/show_ticket', methods=["GET", "POST"])
def show_ticket():
    form = request.form.to_dict()
    res = sys.query_ticket(
        form['datepicker'], form['startSt'], form['toSt'], form['custom-radio-1'])
    return render_template("show_ticket.html", res=res)


@app.route('/instruction', methods=["GET", "POST"])
def instruction():
    return render_template("instruction.html")


@app.route('/ro_run_manager', methods=["GET", "POST"])
def ro_run_manager():
    return render_template("manager.html")


@app.route('/add_train', methods=["GET", "POST"])
def add_train():
    form = request.form.to_dict()
    sys.add_train(form['add'])
    return render_template("manager.html")


@app.route('/delete_train', methods=["GET", "POST"])
def delete_train():
    form = request.form.to_dict()
    sys.delete_train(form['delete'])
    return render_template("manager.html")


@app.route('/release_train', methods=["GET", "POST"])
def release_train():
    form = request.form.to_dict()
    sys.release_train(form['release'])
    return render_template("manager.html")


@app.route('/to_qeury_transfer', methods=["GET", "POST"])
def to_query_transfer():
    return render_template("query_transfer.html")


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=8888)

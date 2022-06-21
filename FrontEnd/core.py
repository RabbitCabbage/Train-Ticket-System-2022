import subprocess
from time import sleep

cnt = 10000


class System:
    def __init__(self, backend):
        self.backend = backend
        self.pipe = subprocess.Popen(
            backend, bufsize=1024, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def __del__(self):
        self.write("exit")

    def read(self):
        ret = ''
        print('read...')
        while True:
            nowline = self.pipe.stdout.readline().decode('UTF-8')
            print(nowline)
            if nowline == "__end__\n":
                break
            else:
                ret += nowline
        return ret

    def write(self, str):
        print(str)
        self.pipe.stdin.write((str + '\n').encode('UTF-8'))
        self.pipe.stdin.flush()

    def login(self, UserName, Passwd):
        cmd = '[0]' + ' login ' + ' -u ' + UserName + ' -p ' + Passwd
        self.write(cmd)
        res = self.read()
        print("res = ")
        print(res)
        list = res.split('\n')
        print(list)
        return list

    def query_self_profile(self, username):
        cmd = '[0] query_profile -c admin -u ' + username
        print(cmd)
        self.write(cmd)
        res = self.read()
        list = res.split(' ')
        if list[3] == '10\n':
            list[3] = "超级用户，拥有管理员权限"
        else:
            list[3] = "普通用户，呜呜"
        return list

    def register(self, UserName, Name, Passwd, Mail):
        cmd = '[1] add_user -c admin -u ' + UserName + ' -p ' + \
            Passwd + ' -n ' + Name + ' -m ' + Mail + ' -g  1'
        self.write(cmd)
        res = self.read()
        print(res)
        list = res.split('\n')
        if list[0] == '-1':
            if UserName == 'admin':
                list[1] = "嘿嘿，admin是管理员哦"
                return False, list
            else:
                return False, list
        else:
            return True, ''

    def query_user(self, UserName):
        cmd = '[0] query_profile -c admin -u ' + UserName
        print(cmd)
        self.write(cmd)
        res = self.read()
        list = res.split('\n')
        if list[0] == '-1':
            print(list)
            return False, list[1:]
        else:
            list = res.split(' ')
            print(list)
            if list[3] == '10\n':
                list[3] = "超级用户，拥有管理员权限"
            else:
                list[3] = "普通用户，呜呜"
            return True, list

    def modify_profile(self, target_user, new_realname, new_email, new_password):
        cmd = '[0] modify_profile -c admin -u ' + \
            target_user + ' -n ' + new_realname
        cmd += ' -m ' + new_email + ' -p ' + new_password
        self.write(cmd)
        res = self.read()
        list = res.split('\n')
        if list[0] == '-1':
            print(list)
            return False, list[1:]
        else:
            list = res.split(' ')
            print(list)
            if list[3] == '10\n':
                list[3] = "超级用户，拥有管理员权限"
            else:
                list[3] = "普通用户，呜呜"
            return True, list

    def query_order(self, username):
        cmd = '[0] query_order -u ' + username
        self.write(cmd)
        res = self.read()
        list = res.split('\n')
        for i in range(1, len(list)):
            list[i] = list[i].split(' ')
        return list

    def refund_ticket(self, username, number):
        cmd = '[0] refund_ticket -u ' + username + ' -n ' + number
        self.write(cmd)
        res = self.read()
        res = res.split('\n')
        if res[0] == '0':
            cmd = '[0] query_order -u ' + username
            self.write(cmd)
            res1 = self.read()
            list = res1.split('\n')
            for i in range(1, len(list)):
                list[i] = list[i].split(' ')
            return True, list
        else:
            cmd = '[0] query_order -u ' + username
            self.write(cmd)
            res1 = self.read()
            list = res1.split('\n')
            for i in range(1, len(list)):
                list[i] = list[i].split(' ')
            return False, list, res[1]

    def buy_ticket(self, username, trainID, num, in_date, ff, tt):
        date = in_date.split('/')
        date = date[0] + '-' + date[1]
        global cnt
        cnt = cnt + 1
        cmd = '[' + str(cnt) + '] ' + 'buy_ticket -u ' + username + ' -i ' + trainID + \
            ' -d ' + date + ' -n ' + num + ' -f ' + ff + ' -t ' + tt
        self.write(cmd)
        res = self.read()
        res = res.split('\n')
        if res[0] == '-1':
            return False, res[1]
        else:
            return True, "您本次购买的总共票价是：" + res[0]

    def query_train(self, in_date, trainID):
        date = in_date.split('/')
        date = date[0] + '-' + date[1]
        cmd = '[0] query_train ' + ' -i ' + trainID + ' -d ' + date
        self.write(cmd)
        res = self.read()
        res = res.split('\n')
        if res[0] == '-1':
            return False, res[1]
        else:
            list = res
            for i in range(1, len(list)):
                list[i] = list[i].split(' ')
            print(list)
            return True, list

    def query_ticket(self, in_date, ss, tt, opt):
        date = in_date.split('/')
        date = date[0] + '-' + date[1]
        cmd = '[0] query_ticket -s ' + ss + ' -t ' + tt + ' -d ' + date
        if opt == "time":
            cmd += ' -p time'
        else:
            cmd += ' -p cost'
        self.write(cmd)
        res = self.read()
        list = res.split('\n')
        for i in range(1, len(list)):
            list[i] = list[i].split(' ')
        return list

    def add_train(self,cmd):
        cmd = '[0] add_train ' + cmd
        self.write(cmd)
        self.read()
        return

    def release_train(self,cmd):
        cmd = '[0] release_train ' + cmd
        self.write(cmd)
        self.read()
        return

    def delete_train(self,cmd):
        cmd = '[0] delete_train ' + cmd
        self.write(cmd)
        self.read()
        return
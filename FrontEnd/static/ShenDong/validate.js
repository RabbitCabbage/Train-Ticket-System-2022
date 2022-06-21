function register_validateForm() {
    // alert("register validate!");
    var username = document.querySelector("input[name='username']").value;
    var password = document.querySelector("input[name='password']").value;
    var email = document.querySelector("input[name='emailaddr']").value;
    var realname = document.querySelector("input[name='realname']").value;
    // if(username=="Username"){
    //     alert("placerholder");
    // }
    if (username == null || username == "") {
        alert("用户名必须填写");
        return false;
    } else if (!isNaN(username[0]) || username[0] == '_') {
        alert("用户名只能是字母开头");
        return false;
    }
    else if (username.length > 20 || username.length <= 3) {
        alert("用户名长度必须在4到20之间");
        return false;
    } else {
        for (var i = 0; i < username.length; ++i) {
            if (isNaN(username[i]) && !(username[i] >= 'a' && username[i] <= 'z') && !(username[i] >= 'A' && username[i] <= 'Z') && username[i] != '_') {
                alert("合法用户名只能含有数字、字母、下划线");
                return false;
            }
        }
    }
    if (password == null || password == "") {
        alert("密码必须填写");
        return false;
    } else if (password.length > 30 || password.length <= 3) {
        alert("密码长度必须在4到30之间");
        return false;
    }
    if (realname == null || realname == "") {
        alert("真实姓名必须填写");
        return false;
    }

    if (email == null || email == "") {
        alert("邮箱必须填写");
        return false;
    } else if (email.length > 30) {
        alert("邮箱地址长度不能大于30");
        return false;
    } else {
        for (var i = 0; i < email.length; ++i) {
            if (isNaN(email[i]) && !(email[i] >= 'a' && email[i] <= 'z') && !(email[i] >= 'A' && email[i] <= 'Z') && email[i] != '@' && email[i] != '.') {
                alert("合法邮箱地址只能含有数字、字母、@和.");
                return false;
            }
        }
    }
}
function login_validateForm() {
    // alert("register validate!");
    var username = document.querySelector("input[name='username']").value;
    var password = document.querySelector("input[name='password']").value;
    if (username == null || username == "") {
        alert("用户名必须填写");
        return false;
    } else if (!isNaN(username[0]) || username[0] == '_') {
        alert("用户名只能是字母开头");
        return false;
    }
    else if (username.length > 20 || username.length <= 3) {
        alert("用户名长度必须在4到20之间");
        return false;
    } else {
        for (var i = 0; i < username.length; ++i) {
            if (isNaN(username[i]) && !(username[i] >= 'a' && username[i] <= 'z') && !(username[i] >= 'A' && username[i] <= 'Z') && username[i] != '_') {
                alert("合法用户名只能含有数字、字母、下划线");
                return false;
            }
        }
    }
    if (password == null || password == "") {
        alert("密码必须填写");
        return false;
    } else if (password.length > 30 || password.length <= 3) {
        alert("密码长度必须在4到30之间");
        return false;
    }
}


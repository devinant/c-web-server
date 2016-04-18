var ajax = function() {
    this.xhr = new XMLHttpRequest();
};

ajax.prototype.send = function(method, url, callback) {
    this.xhr.open(method, url, false);
    this.xhr.onreadystatechange = function() { callback(this.xhr);}.bind(this);

    if (method === 'POST') this.xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');

    this.xhr.send();
    this.xhr.abort();
};

var should = function(code) {
    return function(response) { if (response.status === code) document.getElementById('test-' + code).innerHTML = "OK"; }
};

var test404 = new ajax();
var test501 = new ajax();

test404.send("GET",  "/get.html",  should(404));
test501.send("POST", "/post.html", should(501));
module.exports = function (sails) {

    var exec = require('./lib/exec.js');

    gladys.on('ready', function () {
        sendList();
    });

    return {
        exec: exec,
        sendList: sendList
    };
};
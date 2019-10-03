const Promise = require('bluebird');
const Bottleneck = require('bottleneck');

let retry = 2;

// wait time between signals in ms
const waitTimeBetweenSignals = 100;

// we limit this module at 1 radioemission per 200ms
const limiter = new Bottleneck({
	maxConcurrent: 1,
	minTime: 400
});

module.exports = function (params) {

	// if module serial is not present, we cannot contact the arduino
	if (!gladys.modules.serial || typeof gladys.modules.serial.sendCode !== 'function') {
		sails.log.error(`You need to install the serial module in Gladys.`);
		return Promise.reject(new Error('DEPENDENCY_MISSING'));
	}

	var code = parseInt(params.deviceType.identifier);

	//Récupération de la fonction à exécuter par l'Arduino
	var methode = params.deviceType.protocol;

	//Code Chacon allumage/extinction spécifique à la télécommande
	if (methode == "sendChaconCode") {
		code += 16 * parseInt(params.state.value);
	}

	//Détermination du nombre d'essai
	switch (methode) {
		case "sendChaconCode":
			retry = 3;
			break;
		case "sendRadioCode":
			retry = 3;
			break;
		case "sendIR_LED":
			retry = 1;
			break;
		case "sendIR_TELE":
			retry = 0;
			break;
	}

	var promises = [];
	for (var i = 0; i <= retry; i++) {
		setTimeout(function () {
			promises.push(limiter.schedule(gladys.modules.serial.sendCode, `{"function_name":"${methode}","code":"${code}"}%`));
		}, i * waitTimeBetweenSignals);
	}

	return Promise.all(promises);
};

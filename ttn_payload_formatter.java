function Decoder(bytes, port) {
    var decoded = {};
    var str = String.fromCharCode.apply(null, bytes);
    var data = str.split(',');

    decoded.latitude = parseFloat(data[0]);
    decoded.longitude = parseFloat(data[1]);
    decoded.temperature = parseFloat(data[2]);
    decoded.humidity = parseFloat(data[3]);
    decoded.pressure = parseFloat(data[4]);
    decoded.altitude = parseFloat(data[5]);

    var timeComponents = data[6].split(':');
    var hour = parseInt(timeComponents[0]) + 4; // Adjust for UTC
    var minute = parseInt(timeComponents[1]);
    var second = parseInt(timeComponents[2]);
    decoded.time = hour.toString().padStart(2, '0') + ':' + minute.toString().padStart(2, '0') + ':' + second.toString().padStart(2, '0') + ' UTC';

    decoded.date = data[7] + '/' + data[8] + '/' + data[9];

    return decoded;
}

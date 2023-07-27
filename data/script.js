const connectionStatus = document.getElementById('status');
const connectionAction = document.getElementById('action');
const connectionCountdown = document.getElementById('countdown');
const switches = document.querySelectorAll('.switch');
const buildings = [
    document.getElementById('casa1'),
    document.getElementById('catavento'),
    document.getElementById('casa2')
];

let socket;
let timer;
let leds = [];

function startTimer(time, callbackAction) {
    
    clearInterval(timer);

    let seconds = time;
    const decreaseTimer = () => {
        if (socket.readyState === 1) {
            clearInterval(timer);
            return;
        }
        seconds--;
        connectionCountdown.textContent = seconds.toString();
        if (seconds < 1) {
            connectionCountdown.textContent = '';
            clearInterval(timer);
            callbackAction();
        }
    }
    timer = setInterval(decreaseTimer, 1000);
}

function newSocket() {
    connectionStatus.textContent = 'DESCONECTADO';
    connectionStatus.style.color = 'red';
    connectionAction.textContent = 'Estabelecendo conexão... ';
    socket = new WebSocket('ws://192.168.4.1:81');
    socket.binaryType = "arraybuffer";
    socket.addEventListener('open', socketOpen, false);
    socket.addEventListener('close', socketError, false);
    socket.addEventListener('message', socketReceive, false);
    socket.addEventListener('error', socketError, false);
    startTimer(6, () => socket.close());
}

function socketError(error) {
    connectionStatus.textContent = 'FALHA NA CONEXÃO';
    connectionStatus.style.color = 'red';
    connectionAction.textContent = 'Aguardando nova tentativa de conexão... '
    startTimer(11, newSocket);
}

function socketOpen() {
    connectionStatus.textContent = 'CONECTADO';
    connectionStatus.style.color = 'green';
    connectionAction.textContent = '';
    connectionCountdown.textContent = '';
}

function socketReceive({data}) {
    const receivedData = new DataView(data);

    for (let index = 0; index < receivedData.byteLength; index++) {
        leds[index] = receivedData.getUint8(index);
    }

    updateGUI();
}

function updateGUI() {
    for (let index = 0; index < leds.length; index++) {
        switches[index].checked = leds[index] === 255;
        if (buildings[index].classList.contains('house')) {
            leds[index] === 255 ? buildings[index].classList.remove('dark') : buildings[index].classList.add('dark');
        } else
        if (buildings[index].classList.contains('windvane')) {
            leds[index] === 255 ? buildings[index].classList.remove('paused') : buildings[index].classList.add('paused');
        }
    }
}

function toggleLed(event) {
    event.preventDefault();
    const origin = Number(event.target.dataset.origin);
    const buffer = new ArrayBuffer(2);
    const payload = new Uint8Array(buffer);

    payload.set([origin, leds[origin] === 0 ? 255 : 0], 0);
    socket.readyState === 1 ? socket.send(payload) : alert('Sem conexão com o servidor!');
}
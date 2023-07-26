const connectionStatus = document.getElementById('status');
const switches = document.querySelectorAll('.switch');
const buildings = [document.getElementById('casa1'), document.getElementById('catavento'), document.getElementById('casa2')];
let socket;
let leds = [];
let timer = 0;
let timerInterval = null;

function startTimer() {
    timer = 6;
    timerInterval = setInterval(addTimer, 1000);
    connectionStatus.textContent = 'Desconectado!';
    connectionStatus.style.color = 'red';
}

function addTimer() {
    timer--;
    connectionStatus.innerHTML = `Desconectado!<br><span style="color: yellow">Tentando reconexão em ${timer} segundos...</span>`;
    if (timer < 0) {
        clearInterval(timerInterval);
        connectionStatus.textContent = 'Reconectando...';
        connectionStatus.style.color = 'yellow';
        onPageLoad();
    }
}

function onPageLoad() {
    connectionStatus.textContent = 'Conectando...';
    connectionStatus.style.color = 'yellow';
    socket = new WebSocket('ws://192.168.4.1:81');
    socket.binaryType = "arraybuffer";
    socket.addEventListener('open', socketOpen, false);
    socket.addEventListener('message', socketReceive, false);
    socket.addEventListener('close', socketClose, false);
    socket.addEventListener('error', socketError, false);
}

function socketError(error) {
    connectionStatus.textContent = `Falha na conexão: ${error}`;
    connectionStatus.stylecolor = 'red';
}

function socketClose() {
    connectionStatus.textContent = 'Desconectado!';
    connectionStatus.style.color = 'red';
    startTimer();
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

function socketOpen() {
    connectionStatus.textContent = 'Conectado.';
    connectionStatus.style.color = 'green';
}

function toggleLed(event) {
    event.preventDefault();
    const origin = Number(event.target.dataset.origin);
    const buffer = new ArrayBuffer(2);
    const payload = new Uint8Array(buffer);

    payload.set([origin, leds[origin] === 0 ? 255 : 0], 0);
    socket.readyState === 1 ? socket.send(payload) : alert('Sem conexão com o servidor!');
}
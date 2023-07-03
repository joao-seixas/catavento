const sliders = document.querySelectorAll('.range');
const connectionStatus = document.getElementById('status');
let socket;
let leds = '';
let timer = 0;
let timerInterval = null;

sliders.forEach((slider, index) => slider.addEventListener('change', sliderChange, false));

function sliderChange({target}) {
    console.log(target.value);
}

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
    try {
        socket = new WebSocket('ws://192.168.4.1:81');
        socket.addEventListener('open', socketOpen, false);
        socket.addEventListener('message', socketReceive, false);
        socket.addEventListener('close', socketClose, false);
        socket.addEventListener('error', socketError, false);
    }
    catch (error) {
        socketError(error);
    }
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
    console.log(data);
    // floors.forEach((floor, index) => {
    //     if (floor[0].classList.contains('led-off') && data[index] == '1') {
    //         floor[0].classList.remove('led-off');
    //         floor[1].classList.remove('led-off');
    //     }
    //     if (!floor[0].classList.contains('led-off') && data[index] == '0') {
    //         floor[0].classList.add('led-off');
    //         floor[1].classList.add('led-off');
    //     }
    // });
}

function socketOpen() {
    connectionStatus.textContent = 'Conectado.';
    connectionStatus.style.color = 'green';
    console.log(socket);
}

function toggleLed({target: {dataset, checked}}) {
    let origin = dataset.origin;
    let itemSate = sliders[Number(origin)].value.padStart(3, '0');
    let payload = origin + itemSate;

    console.log(origin);
    console.log(itemSate);
 
    socket.readyState === 1 ? socket.send(payload) : alert('Sem conexão com o servidor!');
}
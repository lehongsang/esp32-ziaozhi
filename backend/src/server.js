const express = require('express');
const http = require('http');
const cors = require('cors');
const path = require('path');
const url = require('url');
const routes = require('./routes');
const { createMqttWsServer } = require('./mqtt');
const { attachCallRelayServer } = require('./call_relay');

const app = express();
const httpPort = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Swagger Documentation
const swaggerUi = require('swagger-ui-express');
const swaggerDocument = require('./swagger.json');

app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerDocument));
app.get('/api/swagger.json', (req, res) => res.json(swaggerDocument));

// Serve API Routes
app.use('/api', routes);

// Serve Static Frontend Dashboard
app.use(express.static(path.resolve(__dirname, '../public')));

// Catch-all
app.get('*', (req, res) => {
    res.sendFile(path.resolve(__dirname, '../public/index.html'));
});

const server = http.createServer(app);

// Initialize WebSocket Servers
const mqttWss = createMqttWsServer();
const callWss = attachCallRelayServer(server);

// Multiplex HTTP Upgrade requests by path
server.on('upgrade', (request, socket, head) => {
    const pathname = url.parse(request.url).pathname;

    if (pathname === '/mqtt') {
        mqttWss.handleUpgrade(request, socket, head, (ws) => {
            mqttWss.emit('connection', ws, request);
        });
    } else if (pathname === '/call') {
        callWss.handleUpgrade(request, socket, head, (ws) => {
            callWss.emit('connection', ws, request);
        });
    } else {
        socket.destroy();
    }
});

server.listen(httpPort, () => {
    console.log(`\n======================================================`);
    console.log(`🚀 MB BUDDY LIVE SYNC & CALL SERVER READY!`);
    console.log(`🌐 Web Parent Dashboard: http://localhost:${httpPort}`);
    console.log(`📑 Swagger API Docs:     http://localhost:${httpPort}/api-docs`);
    console.log(`📡 MQTT Broker (TCP):    Port 1883`);
    console.log(`🌐 MQTT Broker (WS):     ws://localhost:${httpPort}/mqtt`);
    console.log(`📞 Voice Call Relay:     ws://localhost:${httpPort}/call`);
    console.log(`🔌 REST API Base URL:    http://localhost:${httpPort}/api`);
    console.log(`======================================================\n`);
});

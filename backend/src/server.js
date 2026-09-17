const express = require('express');
const http = require('http');
const cors = require('cors');
const path = require('path');
const routes = require('./routes');
const { attachWebSocketServer } = require('./mqtt');

const app = express();
const httpPort = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Serve API Routes
app.use('/api', routes);

// Serve Static Frontend Dashboard
app.use(express.static(path.resolve(__dirname, '../public')));

// Catch-all
app.get('*', (req, res) => {
    res.sendFile(path.resolve(__dirname, '../public/index.html'));
});

const server = http.createServer(app);
attachWebSocketServer(server);

server.listen(httpPort, () => {
    console.log(`\n======================================================`);
    console.log(`🚀 MB BUDDY LIVE SYNC BACKEND READY!`);
    console.log(`🌐 Web Test Dashboard:  http://localhost:${httpPort}`);
    console.log(`📡 MQTT Broker (TCP):   Port 1883`);
    console.log(`🌐 MQTT Broker (WS):    ws://localhost:${httpPort}/mqtt`);
    console.log(`🔌 REST API Base URL:   http://localhost:${httpPort}/api`);
    console.log(`======================================================\n`);
});

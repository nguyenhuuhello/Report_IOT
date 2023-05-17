//Sử dụng express để tạo route
const express = require('express')
const app = express()

//Dùng http để tạo http server
const http = require('http')
const server = http.createServer(app)

//Sử dụng socket.io để viết websocket
const{Server} = require('socket.io')
const io = new Server(server)
// const io = require('socket.io')(server, {
//     transport: ['websocket'],
//     timeout: 3000
// });

const clients = {};

//tạo đường dẫn web trả về
//Đường dẫn này sẽ render trang web điều khiển của chúng ta
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
})


//Tạo kết nối websocket
io.on('connection', (socket) => {
    console.log('Client connected');

    //Tạo kênh nhận lời chào từ esp
    socket.on('user-chat', message => {
        //In ra lời chào ở server
        console.log(message)
        //gửi tới toàn bộ client
        // io.emit('user-chat', message)
        // Lưu trạng thái của client
        clients[socket.id] = true;

        // gửi tới toàn bộ client, trừ client gửi lên server
        Object.keys(clients).forEach(clientId => {
            if (clientId !== socket.id) {
            io.to(clientId).emit('user-chat', message);
            }
        });
    });

    //Tạo kênh điều khiển led
    socket.on('on-btn', led => {
        console.log(led);
        io.emit('on-btn', led)
    });

    //Ngắt két nối
    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
})

server.listen(4200, () => {
    console.log('Listenning on port 4200')
})
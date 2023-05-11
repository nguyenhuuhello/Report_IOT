const express = require('express')
const app = express()

//Dùng http để tạo http server
const http = require('http')
const server = http.createServer(app)

//Sử dụng socket.io
// const{Server} = require('socket.io')
// const io = new Server(server)
const io = require('socket.io')(server, {
    transport: ['polling'],
    timeout: 3000
});

//tạo đường dẫn web trả về
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
})


//Tạo kết nối
io.on('connection', (socket) => {
    console.log('Client connected');

    // dataSchema.find().then(result => {
    //     socket.emit('on-btn', result)
    // })

    socket.on('on-chat', message => {
        console.log(message)
        //gửi tới toàn bộ client
        io.emit('user-chat', message)
    });

    socket.on('on-btn', led => {
        console.log(led);

        io.emit('on-btn', led)
    });

    socket.on('event_name', message => {
        console.log(message);

        io.emit('user-chat', message)
    });

})

server.listen(8080, () => {
    console.log('Listenning on port 4200')
})
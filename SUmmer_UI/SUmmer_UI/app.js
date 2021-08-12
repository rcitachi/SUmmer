uart: 0;
currentData : 0;
var app = {
page : "page1/page1",

    /* app 加载完成触发该函数 */
    onLaunch:function(){
        var that = this;
        uart = pm.openSerialPort({ port: "uart2", baud: 115200 });
        uart.onData(function (data) {
            console.log('uart data : ' + data.toString('hex'));
            console.log('data length : ' + data.length);

            if (currentData)
                currentData.uartUpdate(data);


        });

    },

    /* app 退出触发该函数 */
    onExit:function(){
        if (uart)    
        {        
            uart.close();    
        }
    },

};

App(app);

app = 0;

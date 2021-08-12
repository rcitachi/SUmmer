var page = {

    /* 此方法在第一次显示窗体前发生 */
    onLoad: function (event) {

    },

    /* 此方法展示窗体后发生 */
    onResume: function (event) {

    },

    /* 当前页状态变化为显示时触发 */
    onShow: function (event) {
        currentData=this;
    },

    /* 当前页状态变化为隐藏时触发 */
    onHide: function (event) {

    },

    /* 此方法关闭窗体前发生 */
    onExit: function (event) {

    },
    uartUpdate: function (event) {
//        var point_arr = [];
        var point_x = [];
        var point_y = [];
        var point_z = [];
        var that = this;
        console.log("this is uartUpdate");


        
        for(var i = 0 , a = 0; i < event.length -2;a++ )
        {
            point_x[a] = (event.readUInt16LE(i)*200/70000);
            i+=2;

            point_y[a] = (event.readUInt16LE(i)*200/70000);
            i+=2;

            point_z[a] = (event.readUInt16LE(i)*200/70000);
            i+=2;
            
            
        }

        // for(var i = 0; i < event.length - 2; i += 2 )
        // {
        //    point_arr[i/2] = (event.readUInt16LE(i))*200/70000;
        // }



        // console.log(point_x);
       
        // console.log(point_y);

        // console.log(point_z);     
        function move(pro) {

            var ctx = pm.createCanvasContext('Canvas1', that);
            pro = pro*100;



            // ctx.setFillStyle("0xffff");
            // ctx.moveTo(75,240-point_arr[0]);
            // for (var i = 1 ; i < pro; i++) {
            //      if (i < point_arr.length) {
            //         ctx.lineTo(i+75,240-point_arr[i]);
            //      }
            // }


            ctx.setFillStyle("0xffff");
            ctx.moveTo(75,240-point_x[0]);
            for (var i = 1 ; i < pro; i++) {
                 if (i < point_x.length) {
                    ctx.lineTo(i*4+75,240-point_x[i]);
                 }
            }
            ctx.stroke();
            ctx.setFillStyle("red");
            ctx.moveTo(75,240-point_y[0]);
            for (var i = 1 ; i < pro; i++) {
                 if (i < point_y.length) {
                    ctx.lineTo(i*4+75,240-point_y[i]);
                 }
            }
            ctx.stroke();
            ctx.setFillStyle("#0000FF");
            ctx.moveTo(75,240-point_z[0]);
            for (var i = 1 ; i < pro; i++) {
                 if (i < point_z.length) {
                    ctx.lineTo(i*4+75,240-point_z[i]);
                 }
            }



            // ctx.stroke();
            ctx.draw();
        }

        // console.dir(point_x.length+point_y.length+point_z.length);

        var start = null;
        //动画渲染调用
        function renderCallback(timestamp) {
            if (!start)
                start = timestamp;
            var progress = (timestamp - start) / 1000;
            
            if (progress <= 0.999) {
                move(progress);
            }

            if (0.999 < progress) {
                start = timestamp;
                move(progress);
            }

            if (progress < 1) {
                requestAnimationFrame(renderCallback);
            }
        }
        requestAnimationFrame(renderCallback); // start first frame


         
    },
};

Page(page);

page = 0;

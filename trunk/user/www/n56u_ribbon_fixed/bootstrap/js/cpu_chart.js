/**
 * Created with JetBrains PhpStorm.
 * User: eagle23
 * Date: 30.06.12
 * Time: 16:51

 */
var cpu_chart =
{
    chart: {
        renderTo: 'cpu_chart',
        zoomType: 'x',
        spacingRight: 20
    },
    xAxis: {
        type: 'datetime',
            gapGridLineWidth: 0,
            title: {
            text: null
        },
        labels: {
            formatter: function() {
                return Highcharts.dateFormat('%H:%M:%S', this.value);
            }
        }
    },
    yAxis: {
        title: {
            text: 'CPU %'
        },
        min: 0,
            max: 100,
            startOnTick: false,
            showFirstLabel: false
    },
    plotOptions: {
        areaspline: {lineWidth: 1},
        spline: {lineWidth: 1},
        area: {lineWidth: 1},
        line: {lineWidth: 1}
    },
    legend: {
        enabled: true,
            verticalAlign: 'top',
            floating: true,
            align: 'right'

    },
    rangeSelector: {
        buttons: [{
            count: 1,
            type: 'minute',
            text: '1M'
        }, {
            count: 5,
            type: 'minute',
            text: '5M'
        }, {
            count: 15,
            type: 'minute',
            text: '15M'
        },{
            type: 'all',
            text: 'All'
        }],
            inputEnabled: false,
            selected: 1
    },
    tooltip:{
        xDateFormat: '%H:%M:%S'
    },
    title : {
        text : '<#menu5_8_1#> (%)',
        align: 'left'
    },
    exporting: {
        enabled: true
    },
    series: [{
        type: 'areaspline',
        name: 'Busy',
        gapSize: 5,
        pointInterval: 2 * 1000,
        tooltip: {
            valueSuffix: '%'
        },
        data: (function(){
            var data = [], time = (new Date()).getTime(), i;

            for( i = -600; i <= 0; i++) {
                data.push([
                    parseInt(time/1000)*1000 + i * 2000,
                    0
                ]);
            }

            return data;
        })(),
        fillColor : {
            linearGradient : {
                x1: 0,
                y1: 0,
                x2: 0,
                y2: 1
            },
            stops : [[0, Highcharts.getOptions().colors[0]], [1, 'rgba(0,0,0,0)']]
        },
        threshold: null
    },
    {
        type: 'spline',
        name: 'User',
        gapSize: 5,
        pointInterval: 2 * 1000,
        tooltip: {
            valueSuffix: '%'
        },
        data: (function(){
            var data = [], time = (new Date()).getTime(), i;

            for( i = -600; i <= 0; i++) {
                data.push([
                    parseInt(time/1000)*1000 + i * 2000,
                    0
                ]);
            }

            return data;
        })(),
        threshold: null
    },
    {
        type: 'spline',
        name: 'System',
        gapSize: 5,
        pointInterval: 2 * 1000,
        tooltip: {
            valueSuffix: '%'
        },
        data: (function(){
            var data = [], time = (new Date()).getTime(), i;

            for( i = -600; i <= 0; i++) {
                data.push([
                    parseInt(time/1000)*1000 + i * 2000,
                    0
                ]);
            }

            return data;
        })(),
        threshold: null
    },
    {
        type: 'spline',
        name: 'Sirq',
        gapSize: 5,
        pointInterval: 2 * 1000,
        tooltip: {
            valueSuffix: '%'
        },
        data: (function(){
            var data = [], time = (new Date()).getTime(), i;

            for( i = -600; i <= 0; i++) {
                data.push([
                    parseInt(time/1000)*1000 + i * 2000,
                    0
                ]);
            }

            return data;
        })(),
        threshold: null
    }]
};
/**
 * Created with JetBrains PhpStorm.
 * User: eagle23
 * Date: 30.06.12
 * Time: 16:51

 */
var mem_chart =
{
    chart: {
        renderTo: 'mem_chart',
        zoomType: 'x',
        animation: false,
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
            text: 'Memory (MB)'
        },
        min: 0,
        max: 128,
        startOnTick: false,
        showFirstLabel: false
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
        xDateFormat: '%H:%M:%S',
        valueSuffix: ' MB',
        valueDecimals: 2
    },
    title : {
        text : '<#menu5_8_2#> (MB)',
        align: 'left'
    },
    exporting: {
        enabled: true
    },
    series: [{
        type: 'spline',
        name: 'Used',
        gapSize: 5,
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
        name: 'Buffers',
        gapSize: 5,
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
        name: 'Cached',
        gapSize: 5,
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
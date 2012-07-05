/**
 * Created with JetBrains PhpStorm.
 * User: eagle23
 * Date: 30.06.12
 * Time: 16:51
 */

var network_chart_template =
{
    chart: {
        renderTo: 'network_chart', // this is will be redefined
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
            text: 'Bandwidth (KB/s)'
        },
        min: 0,
        startOnTick: false,
        showFirstLabel: false
    },
    plotOptions: {
        areaspline: {
                lineWidth: 1,
                fillOpacity: 0.3
        }
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
        text : 'Network traffic: ' // will be replaced
    },
    exporting: {
        enabled: true
    },
    series: [{
        type: 'areaspline',
        name: 'Downlink',
        color: '#FF9000',
        gapSize: 5,
        pointInterval: 2 * 1000,
        tooltip: {
            valueSuffix: ' KB/s'
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
        type: 'areaspline',
        name: 'Uplink',
        color: '#003EBA',
        gapSize: 5,
        pointInterval: 2 * 1000,
        tooltip: {
            valueSuffix: ' KB/s'
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
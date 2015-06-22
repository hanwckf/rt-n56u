/**
 * Created with JetBrains PhpStorm.
 * User: eagle23
 * Date: 30.06.12
 * Time: 16:54
 * To change this template use File | Settings | File Templates.
 */
Highcharts.theme = {
    colors: ['#058DC7', '#50B432', '#ED561B', '#DDDF00', '#24CBE5', '#64E572', '#FF9655', '#FFF263', '#6AF9C4'],
    chart: {
        backgroundColor: {
            linearGradient: [0, 0, 500, 500],
            stops: [
                [0, 'rgb(255, 255, 255)'],
                [1, 'rgb(240, 240, 255)']
            ]
        },
        borderRadius: 5,
        borderWidth: 1,
        plotBackgroundColor: 'rgba(255, 255, 255, .9)',
        plotShadow: true,
        plotBorderWidth: 1
    },
    title: {
        style: {
            color: '#000',
            fontFamily: '"Trebuchet MS", Verdana, sans-serif',
            fontWeight: 'bold',
            fontSize: '16px'
        }
    },
    subtitle: {
        style: {
            color: '#666',
            fontFamily: '"Trebuchet MS", Verdana, sans-serif',
            fontWeight: 'bold',
            fontSize: '12px'
        }
    },
    xAxis: {
        gridLineWidth: 0,
        lineWidth: 0,
        lineColor: '#000',
        tickWidth: 1,
        tickLength: 4,
        tickColor: '#999',
        labels: {
            y: 15,
            autoRotation: false,
            style: {
                color: '#888',
                fontFamily: '"Trebuchet MS", Verdana, sans-serif',
                fontSize: '11px'
            }
        },
        title: {
            style: {
                color: '#6D869F',
                fontFamily: '"Trebuchet MS", Verdana, sans-serif',
                fontWeight: 'bold',
                fontSize: '12px'
            }
        }
    },
    yAxis: {
        gridLineWidth: 1,
        minorGridLineWidth: 0,
        minorTickInterval: 'auto',
        lineWidth: 0,
        lineColor: '#000',
        tickWidth: 0,
        tickColor: '#999',
        labels: {
            x: 2,
            align: 'left',
            style: {
                color: '#888',
                fontFamily: '"Trebuchet MS", Verdana, sans-serif',
                fontSize: '11px'
            }
        },
        title: {
            style: {
                color: '#6D869F',
                fontFamily: '"Trebuchet MS", Verdana, sans-serif',
                fontWeight: 'bold',
                fontSize: '12px'
            }
        }
    },
    plotOptions: {
        series: {
            shadow: false
        }
    },
    tooltip: {
        borderWidth: 1,
        borderRadius: 3,
        shadow: false
    },
    legend: {
        borderWidth: 1,
        borderRadius: 3,
        itemStyle: {
            color: '#333',
            fontFamily: '"Trebuchet MS", Verdana, sans-serif',
            fontWeight: 'normal',
            fontSize: '12px'
        },
        itemHoverStyle: {
            color: 'gray'
        }
    },
    navigator: {
        margin: 10
    },
    labels: {
        style: {
            color: '#99b'
        }
    }
};

// Apply the theme
var highchartsOptions = Highcharts.setOptions(Highcharts.theme);

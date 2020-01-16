import signal
import csv
import tempfile

import rclpy
from rclpy.node import Node

from metrics_statistics_msgs.msg import MetricsMessage, StatisticDataType


class MetricsCollectorSubscriber(Node):

    def __init__(self):
        super().__init__('metrics_collector_subscriber')
        signal.signal(signal.SIGINT, self.exit_gracefully)
        signal.signal(signal.SIGTERM, self.exit_gracefully)
        self.subscription = self.create_subscription(
            MetricsMessage,
            'system_metrics',
            self.metrics_collector_callback,
            10)
        self.subscription  # prevent unused variable warning

        self.list_of_statistics = ["minimun", "maximum", "average", "std_dev"];

        self.process_cpu = dict( zip(self.list_of_statistics, [ [] for i in range(len(self.list_of_statistics)) ]) )
        self.system_cpu = dict( zip(self.list_of_statistics, [ [] for i in range(len(self.list_of_statistics)) ]) )
        self.process_memory = dict( zip(self.list_of_statistics, [ [] for i in range(len(self.list_of_statistics)) ]) )
        self.system_memory = dict( zip(self.list_of_statistics, [ [] for i in range(len(self.list_of_statistics)) ]) )

        self.declare_parameter('log_file')
        self.log_filename = self.get_parameter('log_file').value
        print("log_filename: ", self.log_filename)
        if(self.log_filename == None):
            print("Please define log_file parameter to save the statistics")
            exit(0)

    def change_dict_name(self, list_of_statistics, dict, str):
        # changing keys of dictionary
        for name in list_of_statistics:
            dict[str + name] = dict[name]
            del dict[name]

    def exit_gracefully(self,signum, frame):

        csv_file = self.log_filename
        csv_columns = ["process_cpu_" + str for str in self.list_of_statistics]
        csv_columns = csv_columns + ["system_cpu_" + str for str in self.list_of_statistics]
        csv_columns = csv_columns + ["process_memory_" + str for str in self.list_of_statistics]
        csv_columns = csv_columns + ["system_memory_" + str for str in self.list_of_statistics]

        self.change_dict_name(self.list_of_statistics, self.process_cpu, "process_cpu_")
        self.change_dict_name(self.list_of_statistics, self.process_memory, "process_memory_")
        self.change_dict_name(self.list_of_statistics, self.system_cpu, "system_cpu_")
        self.change_dict_name(self.list_of_statistics, self.system_memory, "system_memory_")
        dict_data = {**self.process_cpu, **self.process_memory, **self.system_memory, **self.system_cpu}

        try:
            with open(csv_file, 'w') as csvfile:
                writer = csv.writer(csvfile, delimiter = "\t")
                writer.writerow(dict_data.keys())
                writer.writerows(zip(*[dict_data[key] for key in dict_data.keys()]))
        except IOError:
            print("I/O error")
        exit(0)

    def fill_dict(self, statistics, dict):
        for metric in statistics:
            if(metric.data_type == StatisticDataType.STATISTICS_DATA_TYPE_AVERAGE):
                dict['average'].append(metric.data)
            if(metric.data_type == StatisticDataType.STATISTICS_DATA_TYPE_STDDEV):
                dict['std_dev'].append(metric.data)
            if(metric.data_type == StatisticDataType.STATISTICS_DATA_TYPE_MINIMUM):
                dict['minimun'].append(metric.data)
            if(metric.data_type == StatisticDataType.STATISTICS_DATA_TYPE_MAXIMUM):
                dict['maximum'].append(metric.data)

    def metrics_collector_callback(self, msg):
        if(msg.measurement_source_name == 'linuxProcessMemoryCollector'):
            self.fill_dict(msg.statistics, self.process_memory)
        if(msg.measurement_source_name == 'linuxProcessCpuCollector'):
            self.fill_dict(msg.statistics, self.process_cpu)
        if(msg.measurement_source_name == 'linuxCpuCollector'):
            self.fill_dict(msg.statistics, self.system_cpu)
        if(msg.measurement_source_name == 'linuxMemoryCollector'):
            self.fill_dict(msg.statistics, self.system_memory)

def main(args=None):
    rclpy.init(args=args)

    metrics_collector_subscriber = MetricsCollectorSubscriber()

    rclpy.spin(metrics_collector_subscriber)

    metrics_collector_subscriber.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()

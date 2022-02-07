import json
from datetime import datetime

def print_data():
    file = open('type200-example.json', 'r')
    data = json.load(file)

    #examples of usage
    print('Experiment name: {}'.format(data['exper_name']))

    print('record_id + version_no: {}'.format(data['record_id']+data['version_no']))

    print('date scantime: {}'.format(data['date']['scantime']))

    print(datetime.strptime(data['time_stamp'], '%Y-%m-%dT%H:%M:%S.%f%z'))

    file.close()

def write_to_data():
    file = open('test.json', 'w')
    print(json.dumps({"c": 0, "b": 0, "a": 0}, sort_keys=True, indent=4))

    json.dump({"c": 0, "b": 0, "a": 0}, file, sort_keys=True, indent=4)
    
    file.close()


if __name__ == "__main__":
    #print_data()
    write_to_data()

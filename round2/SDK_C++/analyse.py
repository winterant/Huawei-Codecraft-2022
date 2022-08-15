from matplotlib import pyplot as plt
import pandas

# data_path="dataset/data"
data_path = "dataset/pressure0.3-t200-n100-m20"
# data_path = "dataset/pressure0.3-t8926-n135-m35"

demand = pandas.read_csv(data_path + "/demand.csv")

for i, c in enumerate(demand.columns):
    plt.plot(demand.index, demand[c])
    if(i == 3):
        break
plt.show()

qos = pandas.read_csv(data_path + "/qos.csv")
qos = (qos.drop(columns=['site_name']).to_numpy() < 400).astype(int)
plt.matshow(qos)
plt.show()

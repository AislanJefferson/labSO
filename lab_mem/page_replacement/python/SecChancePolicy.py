from FifoPolicy import FifoPolicy

class SecChancePolicy(FifoPolicy):
    def put(self, frameId):
        self.fifo.append([False, frameId])

    def evict(self):
        leitura,item = self.fifo.pop(0)
        if (not leitura):
            return item
        else:
             self.fifo.append([False,item])
        return self.evict()

    def access(self, frameId, isWrite):
        for frame in self.fifo:
            if(frameId == frame[1]):
                frame[0] = True

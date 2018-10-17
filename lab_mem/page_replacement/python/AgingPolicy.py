from NRUPolicy import NRUPolicy

class AgingPolicy(NRUPolicy):
    def evict(self):
        n = 256 #count maximo eh 255
        frame_to_remove = {}
        for frame in self.list:
            
            if (frame['count'] < n):
                n = frame['count']
                frame_to_remove = frame

        self.list.remove(frame_to_remove)
        return frame_to_remove['frameId']

    def clock(self):
        for frame in self.list:
            frame['count'] = frame['count'] >> 1

    def access(self, frameId, isWrite):
        for frame in self.list:
            if(frameId == frame['frameId']):
                frame['count'] = frame['count'] | 128

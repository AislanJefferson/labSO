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
            add_bit = 0
            if (frame['isRead']):
                add_bit = 128
                frame['isRead'] = False
            frame['count'] = (add_bit | (frame['count'] >> 1))

    def access(self, frameId, isWrite):
        for frame in self.list:
            if(frameId == frame['frameId']):
                frame['isRead'] = True
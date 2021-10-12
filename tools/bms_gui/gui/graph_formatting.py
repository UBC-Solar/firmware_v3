# This file takes care of all the graphing done in the program (voltage & Temperature)

from reading_com_port.parse_information   import parse_float_from_str
def gen_x_values(values, update_interval_in_seconds):
    x_values = []
    for i in range(len(values)):
        x_values.append(i * update_interval_in_seconds)

    return x_values


def points_to_plot(values, how_many_minutes_to_show, update_interval_in_seconds, max_entries=2):
    # if len(values)<=max_entries:
    #
    #     return (values,gen_x_values(values,update_interval_in_seconds))

    # how_many_minutes_present = update_interval_in_seconds*len(values)/60
    #
    # if how_many_minutes_present<how_many_minutes_to_show:
    #     return (values,)

    how_many_seconds_to_show = 60 * how_many_minutes_to_show
    # max_entries //= 2

    entry_num_to_include = (how_many_seconds_to_show // update_interval_in_seconds)
    excluded_number = len(values) - entry_num_to_include
    if excluded_number < 0:
        excluded_number = 0

    if entry_num_to_include < len(values):
        real_values = values[-entry_num_to_include:]
    else:
        real_values = values

    x_intervals = round(len(real_values) / max_entries)
    if x_intervals == 0:
        x_intervals = 1
    i = 0

    final_values = []
    x_values = []

    while i < len(real_values) and len(final_values) < max_entries:
        final_values.append(real_values[i])
        x_values.append((i + excluded_number) * update_interval_in_seconds)
        i += x_intervals

    hedef = 0 + x_intervals
    curav = 0
    total = 0
    alternative = []
    altxvalues = []
    for i in range(len(real_values) + 1):
        if i == hedef:
            hedef += x_intervals
            alternative.append(curav / total)
            altxvalues.append((i + excluded_number) * update_interval_in_seconds)
            if i == len(real_values):
                break
            curav = real_values[i]
            total = 1

        elif i != len(real_values):
            curav += real_values[i]
            total += 1

    # newhedef = x_intervals+0
    # try:
    #     curmin = real_values[0]
    # except:
    #     return ([],[])
    #
    # curmax = real_values[0]
    # curmaxin = 0
    # curminin = 0
    #
    # newalt = []
    # newaltxvalues = []
    #
    # for i in range(len(real_values)):
    #     if i==newhedef:
    #          newhedef+=x_intervals
    #
    #          newaltxvalues.extend(sorted([(curminin+excluded_number)*update_interval_in_seconds,
    #                                (curmaxin+excluded_number)*update_interval_in_seconds]))
    #
    #          if curmin>curmax:
    #              newalt.extend([curmax, curmin])
    #          else:
    #              newalt.extend([curmin,curmax])
    #          if i==len(real_values):
    #              break
    #          curmin = real_values[i]
    #          curmax = real_values[i]
    #          curmaxin = i
    #          curminin = i
    #
    #     elif real_values[i]<curmin:
    #         curmin = real_values[i]
    #
    #         curminin = i
    #
    #     elif real_values[i]>curmax:
    #         curmax = real_values[i]
    #         curmaxin = i

    # make sure the most recent entry is included in the graph:
    # if final_values[-1]!=values[-1]:
    #     final_values.append(values[-1])
    #     x_values.append((len(values)-1)*update_interval_in_seconds)

    # return (alternative,altxvalues)

    return (alternative, altxvalues)


class grapher:

    def __init__(self, array_of_options=None):
        self.goer = 0

        self.total_plots = {}
        if array_of_options is None:
            array_of_options = ["1 minute",
                                "2 minutes",
                                "5 minutes", "15 minutes", "30 minutes", "1 hour", "2 hours", "3 hours",
                                "5 hours",
                                "10 hours"]

        self.last_event = array_of_options[0]

        self.summaries = {

        }
        for option in array_of_options:
            # num = parse_float_from_str(string=option)
            # minutes_time_interval = num
            #
            # if "hour" in option:
            #     minutes_time_interval *= 60
            #
            # minutes_time_interval = int(minutes_time_interval)

            self.summaries[option] = [[], []]
            self.total_plots[option] = 0

    def plot_point(self, values, how_many_minutes_to_show, num_of_entries, update_interval_in_seconds, event,
                   max_entries=100):

        how_many_seconds_to_show = 60 * how_many_minutes_to_show
        entry_num_to_include = (how_many_seconds_to_show // update_interval_in_seconds)

        # if num_of_entries<max_entries*entry_num_to_include:


        excluded_number = num_of_entries - entry_num_to_include

        x_intervals = round(entry_num_to_include / max_entries)
        if x_intervals == 0:
            x_intervals = 1



        if event not in self.summaries or\
                self.last_event!=event or  \
                self.total_plots[event]<2:
                # len(values)-len(self.summaries[event][0])*x_intervals<x_intervals*2:


            event=event.replace(" d","")
            self.last_event = event

            self.total_plots[event] = 0

            self.summaries[event]  = [[],[]]
            entry_num_to_include = int(how_many_seconds_to_show // update_interval_in_seconds)
            excluded_number = int(len(values) - entry_num_to_include)
            if excluded_number < 0:
                excluded_number = 0

            if entry_num_to_include < len(values):
                # print(entry_num_to_include)
                values = values[-entry_num_to_include:]

#            else:
 #               real_values = values

            for i in range(1+x_intervals,len(values)+1):
                curvalues = values[0:i]

                self.add_new_point(values=curvalues,
                                   how_many_minutes_to_show=how_many_minutes_to_show,
                                   update_interval_in_seconds=update_interval_in_seconds,
                                   num_of_entries = len(curvalues),
                                   max_entries = max_entries,
                                   event = event )



            # print()
            # print(len(self.summaries[event][0]))


        else:
            self.last_event = event
            self.add_new_point(values,
                               how_many_minutes_to_show,
                               update_interval_in_seconds,
                               num_of_entries,
                               event,
                            max_entries)

        return self.summaries[event]

    def add_new_point(self, values,
                      how_many_minutes_to_show,
                      update_interval_in_seconds,
                      num_of_entries,
                      event,
                      max_entries=2):

        how_many_seconds_to_show = 60 * how_many_minutes_to_show
        entry_num_to_include = (how_many_seconds_to_show // update_interval_in_seconds)
        excluded_number = num_of_entries - entry_num_to_include

        x_intervals = round(entry_num_to_include / max_entries)
        if x_intervals == 0:
            x_intervals = 1

        # print(event, "dude: ", len(self.summaries[event][0]), how_many_minutes_to_show)






        if num_of_entries - self.total_plots[event] * x_intervals > x_intervals:

            self.total_plots[event] += 1
            curav = 0

            for i in range(1, 1 + x_intervals):
                curav += values[-i]

            curav /= x_intervals
            self.summaries[event][0].append(curav)
            self.summaries[event][1].append(num_of_entries * update_interval_in_seconds)
            if len(self.summaries[event][0]) > max_entries:
                self.summaries[event][0].pop(0)
                self.summaries[event][1].pop(0)

        # return self.summaries[event]

class grapher_manager:
    def __init__(self,how_many_modules=32):
        self.graphers = []
        for i in range(how_many_modules):
            self.graphers.append(grapher())


    def plot_module(self,module_number,values, how_many_minutes_to_show, num_of_entries, update_interval_in_seconds, event,max_entries):
        array_of_options = ["1 minute",
                            "2 minutes",
                            "5 minutes", "15 minutes", "30 minutes", "1 hour", "2 hours", "3 hours",
                            "5 hours",
                            "10 hours"]

        self.graphers[module_number].last_event = event

        thing = self.graphers[module_number].plot_point(values, how_many_minutes_to_show, num_of_entries,
                                                 update_interval_in_seconds, event,
                   max_entries)

        for ev in array_of_options:
            if ev==event:
                continue
            minstoshow=parse_float_from_str(ev)
            if "hour" in array_of_options:
                minstoshow*=60


            self.graphers[module_number].last_event = ev

            self.graphers[module_number].plot_point(
                values,minstoshow,num_of_entries,update_interval_in_seconds,ev
            )

        self.graphers[module_number].last_event = event

        return thing

    def update_all_states(self,module_number,values, num_of_entries, update_interval_in_seconds, event,max_entries):
        array_of_options = ["1 minute",
                            "2 minutes",
                            "5 minutes", "15 minutes", "30 minutes", "1 hour", "2 hours", "3 hours",
                            "5 hours",
                            "10 hours"]


        for ev in array_of_options:

            minstoshow=parse_float_from_str(ev)
            if "hour" in ev:
                minstoshow*=60


            self.graphers[module_number].last_event = ev

            self.graphers[module_number].plot_point(
                values,minstoshow,num_of_entries,update_interval_in_seconds,ev,max_entries=max_entries
            )

            # print(len(self.graphers[module_number].summaries[ev][0]))
        self.graphers[module_number].last_event = event

    def get_state(self, module_number, string_minutes_to_show):

        return self.graphers[module_number].summaries[string_minutes_to_show]

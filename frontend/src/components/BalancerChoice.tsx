import * as React from "react";
import BalancerPolicy from "../misc/BalancerPolicy";
import { Balancers } from "../Constants";

interface BalancerChoiceProps {
  balancerPolicy: BalancerPolicy;
}

interface BalancerChoiceState {
  active: string;
  items: Array<{ key: string; title: string; disabled?: boolean }>;
}

export default class BalancerChoice extends React.Component<
  BalancerChoiceProps,
  BalancerChoiceState
> {
  constructor(props: BalancerChoiceProps) {
    super(props);

    this.props.balancerPolicy.setBalancer("naive");

    this.state = {
      active: this.props.balancerPolicy.getBalancer(),
      items: Balancers
    };
  }

  public handleBalancerChoice(balancer: string) {
    this.props.balancerPolicy.setBalancer(balancer);

    this.setState(state => {
      const newState = {
        active: this.props.balancerPolicy.getBalancer(),
        items: state.items
      };
      return newState;
    });
  }

  public render() {
    const itemList = [];
    for (const item of this.state.items) {
      const active = item.key == this.state.active;
      const handler = item.disabled ? () => false : () => this.handleBalancerChoice(item.key);
      itemList.push(
        <BalancerItem
          key={item.key}
          title={item.title}
          active={active}
          disabled={item.disabled || false}
          handleClick={handler}
        />
      );
    }

    return <div className="list-group balancerChoice">{itemList}</div>;
  }
}

function BalancerItem(props: BalancerItemProps) {
  let classList = "list-group-item list-group-item-action";
  if (props.active) {
    classList += " active";
  }
  if (props.disabled) {
    classList += " disabled";
  }
  return (
    <a href="#" className={classList} onClick={props.handleClick}>
      {props.title}
    </a>
  );
}

interface BalancerItemProps {
  active: boolean;
  disabled: boolean;
  handleClick: (event?: React.MouseEvent<HTMLAnchorElement>) => void;
  title: string;
}

// BalancerItem.defaultProps = {
//   active: false,
//   disabled: false,
//   handleClick: () => {}
// };
